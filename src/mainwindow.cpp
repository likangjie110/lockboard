#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialManager(new SerialManager(this))
    , m_autoConnecting(false)
    , m_currentPortIndex(0)
    , m_currentAddress(1)
    , m_autoConnectTimer(new QTimer(this))
    , m_lastSentMask(0)
{
    ui->setupUi(this);
    setWindowTitle("485锁控板通讯测试工具");
    resize(1200, 800);
    
    setupUI();
    setupConnections();
    
    onRefreshPorts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // ===== 左侧：连接区域 =====
    QGroupBox *connectGroup = new QGroupBox("连接设置");
    QVBoxLayout *connectLayout = new QVBoxLayout();
    
    QHBoxLayout *portLayout = new QHBoxLayout();
    portLayout->addWidget(new QLabel("串口:"));
    m_portCombo = new QComboBox();
    m_portCombo->setMinimumWidth(120);
    portLayout->addWidget(m_portCombo);
    m_refreshButton = new QPushButton("刷新");
    portLayout->addWidget(m_refreshButton);
    connectLayout->addLayout(portLayout);
    
    QHBoxLayout *addressLayout = new QHBoxLayout();
    addressLayout->addWidget(new QLabel("设备地址:"));
    m_addressSpin = new QSpinBox();
    m_addressSpin->setRange(1, 15);
    m_addressSpin->setValue(1);
    addressLayout->addWidget(m_addressSpin);
    connectLayout->addLayout(addressLayout);
    
    m_connectButton = new QPushButton("连接");
    m_connectButton->setMinimumHeight(40);
    connectLayout->addWidget(m_connectButton);
    
    m_autoConnectButton = new QPushButton("自动连接");
    m_autoConnectButton->setMinimumHeight(40);
    m_autoConnectButton->setToolTip("自动扫描串口并识别设备地址");
    connectLayout->addWidget(m_autoConnectButton);
    
    connectLayout->addStretch();
    connectGroup->setLayout(connectLayout);
    connectGroup->setMaximumWidth(250);
    
    // ===== 中间：功能Tab区域 =====
    m_tabWidget = new QTabWidget();
    
    // Tab 1: 版本信息
    QWidget *versionTab = new QWidget();
    QVBoxLayout *versionLayout = new QVBoxLayout(versionTab);
    QPushButton *getVersionBtn = new QPushButton("获取版本号");
    getVersionBtn->setMinimumHeight(40);
    connect(getVersionBtn, &QPushButton::clicked, this, &MainWindow::onGetVersion);
    versionLayout->addWidget(getVersionBtn);
    versionLayout->addStretch();
    m_tabWidget->addTab(versionTab, "版本信息");
    
    // Tab 2: 单锁操作
    QWidget *singleTab = new QWidget();
    QVBoxLayout *singleLayout = new QVBoxLayout(singleTab);
    
    QHBoxLayout *singleLockLayout = new QHBoxLayout();
    singleLockLayout->addWidget(new QLabel("锁号:"));
    QSpinBox *singleLockSpin = new QSpinBox();
    singleLockSpin->setRange(1, 12);
    singleLockSpin->setObjectName("singleLockSpin");
    singleLockLayout->addWidget(singleLockSpin);
    singleLayout->addLayout(singleLockLayout);
    
    QPushButton *openSingleBtn = new QPushButton("开单个锁");
    openSingleBtn->setMinimumHeight(40);
    connect(openSingleBtn, &QPushButton::clicked, this, &MainWindow::onOpenSingleLock);
    singleLayout->addWidget(openSingleBtn);
    
    QPushButton *readSingleBtn = new QPushButton("读单个锁状态");
    readSingleBtn->setMinimumHeight(40);
    connect(readSingleBtn, &QPushButton::clicked, this, &MainWindow::onReadSingleLock);
    singleLayout->addWidget(readSingleBtn);
    
    singleLayout->addStretch();
    m_tabWidget->addTab(singleTab, "单锁操作");
    
    // Tab 3: 批量操作
    QWidget *batchTab = new QWidget();
    QVBoxLayout *batchLayout = new QVBoxLayout(batchTab);
    
    QPushButton *readAllBtn = new QPushButton("读取所有锁状态");
    readAllBtn->setMinimumHeight(40);
    connect(readAllBtn, &QPushButton::clicked, this, &MainWindow::onReadAllLocks);
    batchLayout->addWidget(readAllBtn);
    
    QPushButton *openAllBtn = new QPushButton("开全部锁");
    openAllBtn->setMinimumHeight(40);
    connect(openAllBtn, &QPushButton::clicked, this, &MainWindow::onOpenAllLocks);
    batchLayout->addWidget(openAllBtn);
    
    batchLayout->addStretch();
    m_tabWidget->addTab(batchTab, "批量操作");
    
    // Tab 4: 多锁操作
    QWidget *multiTab = new QWidget();
    QVBoxLayout *multiLayout = new QVBoxLayout(multiTab);
    
    QLabel *multiLabel = new QLabel("选择要操作的锁:");
    multiLayout->addWidget(multiLabel);
    
    QGridLayout *checkLayout = new QGridLayout();
    for (int i = 0; i < 12; ++i) {
        QCheckBox *cb = new QCheckBox(QString("锁%1").arg(i + 1));
        cb->setObjectName(QString("multiCheck_%1").arg(i + 1));
        checkLayout->addWidget(cb, i / 4, i % 4);
    }
    multiLayout->addLayout(checkLayout);
    
    QPushButton *openMultiBtn = new QPushButton("开多个锁(0x15)");
    openMultiBtn->setMinimumHeight(40);
    connect(openMultiBtn, &QPushButton::clicked, this, &MainWindow::onOpenMultipleLocks);
    multiLayout->addWidget(openMultiBtn);
    
    QPushButton *simOpenBtn = new QPushButton("同时开锁(0x18)");
    simOpenBtn->setMinimumHeight(40);
    connect(simOpenBtn, &QPushButton::clicked, this, &MainWindow::onSimultaneousOpen);
    multiLayout->addWidget(simOpenBtn);
    
    multiLayout->addStretch();
    m_tabWidget->addTab(multiTab, "多锁操作");
    
    // Tab 5: 持续输出
    QWidget *outputTab = new QWidget();
    QVBoxLayout *outputLayout = new QVBoxLayout(outputTab);
    
    QHBoxLayout *channelLayout = new QHBoxLayout();
    channelLayout->addWidget(new QLabel("通道:"));
    QSpinBox *channelSpin = new QSpinBox();
    channelSpin->setRange(1, 12);
    channelSpin->setObjectName("channelSpin");
    channelLayout->addWidget(channelSpin);
    outputLayout->addLayout(channelLayout);
    
    QPushButton *outputOnBtn = new QPushButton("持续输出-开(0x16)");
    outputOnBtn->setMinimumHeight(40);
    connect(outputOnBtn, &QPushButton::clicked, this, &MainWindow::onContinuousOutput);
    outputLayout->addWidget(outputOnBtn);
    
    QPushButton *multiOutputBtn = new QPushButton("多通道输出(0x1A)");
    multiOutputBtn->setMinimumHeight(40);
    connect(multiOutputBtn, &QPushButton::clicked, this, &MainWindow::onMultiOutput);
    outputLayout->addWidget(multiOutputBtn);
    
    outputLayout->addStretch();
    m_tabWidget->addTab(outputTab, "持续输出");
    
    // ===== 右侧：状态显示和调试控制台 =====
    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    
    m_lockStatusWidget = new LockStatusWidget();
    rightLayout->addWidget(m_lockStatusWidget);
    
    m_debugConsole = new DebugConsole();
    rightLayout->addWidget(m_debugConsole);
    
    // 组装主布局
    mainLayout->addWidget(connectGroup);
    mainLayout->addWidget(m_tabWidget, 1);
    mainLayout->addWidget(rightWidget, 1);
    
    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectToggle);
    connect(m_autoConnectButton, &QPushButton::clicked, this, &MainWindow::onAutoConnect);
    
    connect(m_serialManager, &SerialManager::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_serialManager, &SerialManager::errorOccurred, this, &MainWindow::onSerialError);
    connect(m_serialManager, &SerialManager::portOpened, [this](const QString &port) {
        m_debugConsole->logInfo(QString("串口 %1 已打开").arg(port));
    });
    connect(m_serialManager, &SerialManager::portClosed, [this]() {
        m_debugConsole->logInfo("串口已关闭");
    });
    connect(m_serialManager, &SerialManager::responseTimeout, this, [this]() {
        m_debugConsole->logError("设备响应超时，请检查：\n1. 设备是否已上电\n2. 设备地址是否正确\n3. RS485连接是否正常");
        QMessageBox::warning(this, "响应超时", 
            "设备在规定时间内未响应\n\n可能原因：\n"
            "• 设备未上电或已断电\n"
            "• 设备地址设置不匹配\n"
            "• RS485线路连接错误（A/B接反）\n"
            "• RS485转换器故障\n\n"
            "建议操作：\n"
            "1. 检查设备电源指示灯\n"
            "2. 确认设备地址拨码开关设置\n"
            "3. 检查RS485接线");
    });
    connect(m_serialManager, &SerialManager::portDisconnected, this, [this]() {
        m_debugConsole->logError("串口设备已断开连接");
        QMessageBox::critical(this, "设备断开", 
            "串口设备已断开连接\n\n可能原因：\n"
            "• USB线缆松动或拔出\n"
            "• RS485转换器断电\n"
            "• 设备驱动异常\n\n"
            "请重新连接设备后再试");
        
        // 自动断开连接并重置UI
        if (m_serialManager->isOpen()) {
            m_serialManager->closePort();
            m_connectButton->setText("连接");
            m_portCombo->setEnabled(true);
            m_lockStatusWidget->resetAllStates();
        }
    });
    
    // 自动连接定时器
    m_autoConnectTimer->setSingleShot(true);
    connect(m_autoConnectTimer, &QTimer::timeout, this, &MainWindow::performAutoConnect);
}

void MainWindow::onRefreshPorts()
{
    m_portCombo->clear();
    QStringList ports = SerialManager::availablePorts();
    m_portCombo->addItems(ports);
    m_debugConsole->logInfo(QString("发现 %1 个串口").arg(ports.size()));
}

void MainWindow::onConnectToggle()
{
    if (m_serialManager->isOpen()) {
        m_serialManager->closePort();
        m_connectButton->setText("连接");
        m_portCombo->setEnabled(true);
        m_addressSpin->setEnabled(true);
        m_lockStatusWidget->resetAllStates();
        m_debugConsole->logInfo("已断开串口连接");
    } else {
        QString portName = m_portCombo->currentText();
        
        if (portName.isEmpty()) {
            QMessageBox::warning(this, "警告", 
                "请先选择串口\n\n点击\"刷新\"按钮扫描可用串口");
            m_debugConsole->logError("未选择串口");
            return;
        }
        
        // 验证设备地址
        quint8 addr = m_addressSpin->value();
        if (addr < 1 || addr > 15) {
            QMessageBox::warning(this, "参数错误", 
                QString("设备地址 %1 超出有效范围\n\n有效范围：1-15\n"
                       "请确认设备拨码开关设置").arg(addr));
            m_debugConsole->logError(QString("无效的设备地址: %1").arg(addr));
            return;
        }
        
        m_debugConsole->logInfo(QString("正在连接串口 %1...").arg(portName));
        
        if (m_serialManager->openPort(portName)) {
            m_connectButton->setText("断开");
            m_portCombo->setEnabled(false);
            m_addressSpin->setEnabled(false);
            m_debugConsole->logInfo(QString("串口连接成功 - 设备地址:%1, 超时设置:%2ms")
                .arg(addr).arg(m_serialManager->getResponseTimeout()));
            
            // 建议先获取版本验证通讯
            m_debugConsole->logInfo("提示：建议先点击\"获取版本号\"验证通讯是否正常");
        } else {
            QMessageBox::critical(this, "连接失败", 
                QString("无法打开串口 %1\n\n可能原因：\n"
                       "• 串口被其他程序占用\n"
                       "• 设备驱动未正确安装\n"
                       "• 没有访问权限\n\n"
                       "请关闭占用串口的程序后重试").arg(portName));
            m_debugConsole->logError(QString("打开串口 %1 失败").arg(portName));
        }
    }
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    m_debugConsole->logReceive(data);
    
    ParseResult result = Protocol485::parseResponse(data);
    if (!result.success) {
        m_debugConsole->logError(QString("数据解析失败: %1").arg(result.errorMsg));
        
        // 对于关键错误，弹窗提示用户
        if (result.errorMsg.contains("包头错误")) {
            m_debugConsole->logError("提示：可能存在总线冲突或设备地址错误");
        } else if (result.errorMsg.contains("校验错误")) {
            m_debugConsole->logError("提示：数据传输可能存在干扰，建议检查RS485线路");
        } else if (result.errorMsg.contains("长度不匹配")) {
            m_debugConsole->logError("提示：接收到不完整的数据帧，可能是通讯不稳定");
        }
        return;
    }
    
    // 如果正在自动连接且收到了有效的版本响应，说明找到了设备
    if (m_autoConnecting && result.command == Command::GetVersion) {
        m_autoConnecting = false;
        m_autoConnectTimer->stop();
        
        QString foundPort = m_portsToTry[m_currentPortIndex];
        quint8 foundAddress = m_currentAddress;
        
        if (result.data.size() >= 2) {
            quint8 major = static_cast<quint8>(result.data[0]);
            quint8 minor = static_cast<quint8>(result.data[1]);
            QString version = QString("V%1.%2").arg(major).arg(minor);
            
            m_debugConsole->logInfo("====== 自动连接成功！======");
            m_debugConsole->logInfo(QString("串口: %1").arg(foundPort));
            m_debugConsole->logInfo(QString("设备地址: %1").arg(foundAddress));
            m_debugConsole->logInfo(QString("设备版本: %1").arg(version));
            
            // 更新UI
            m_portCombo->setCurrentText(foundPort);
            m_addressSpin->setValue(foundAddress);
            m_connectButton->setText("断开");
            m_autoConnectButton->setText("自动连接");
            m_autoConnectButton->setEnabled(true);
            m_connectButton->setEnabled(true);
            
            QMessageBox::information(this, "自动连接成功", 
                QString("已找到设备！\n\n"
                       "串口: %1\n"
                       "设备地址: %2\n"
                       "固件版本: %3\n\n"
                       "设备已连接，可以开始操作")
                .arg(foundPort).arg(foundAddress).arg(version));
        }
        return;
    }
    
    // 根据命令类型处理响应
    switch (result.command) {
        case Command::GetVersion:
            if (result.data.size() >= 2) {
                quint8 major = static_cast<quint8>(result.data[0]);
                quint8 minor = static_cast<quint8>(result.data[1]);
                QString version = QString("V%1.%2").arg(major).arg(minor);
                m_debugConsole->logInfo(QString("设备版本号: %1").arg(version));
                
                // 只有在非自动连接模式下才弹窗
                if (!m_autoConnecting) {
                    QMessageBox::information(this, "设备版本", 
                        QString("固件版本：%1\n\n设备通讯正常").arg(version));
                }
            } else {
                m_debugConsole->logError("版本信息数据长度不足");
            }
            break;
            
        case Command::OpenSingle:
        case Command::ReadSingle:
            // 兼容两种响应格式：
            // 格式1（标准）: 2字节 [锁ID][状态]
            // 格式2（实际设备）: 1字节 [状态]
            if (result.data.size() >= 1) {
                quint8 lockId;
                bool opened;
                
                if (result.data.size() >= 2) {
                    // 格式1：包含锁ID和状态
                    lockId = static_cast<quint8>(result.data[0]);
                    opened = static_cast<quint8>(result.data[1]) == 0x01;
                    
                    // 参数验证
                    if (lockId < 1 || lockId > 12) {
                        m_debugConsole->logError(QString("收到无效的锁ID: %1").arg(lockId));
                        return;
                    }
                } else {
                    // 格式2：只有状态，从发送的命令中获取锁ID
                    // 通过查找最近操作的锁号（从UI控件获取）
                    QSpinBox *spin = nullptr;
                    for (int i = 0; i < m_tabWidget->count(); ++i) {
                        QWidget *tab = m_tabWidget->widget(i);
                        spin = tab->findChild<QSpinBox*>("singleLockSpin");
                        if (spin) break;
                    }
                    
                    if (spin) {
                        lockId = spin->value();
                    } else {
                        m_debugConsole->logError("无法确定锁ID，响应数据格式异常");
                        return;
                    }
                    
                    opened = static_cast<quint8>(result.data[0]) == 0x01;
                    m_debugConsole->logWarning(QString("设备使用简化响应格式（仅状态字节），已自动适配锁%1").arg(lockId));
                }
                
                m_lockStatusWidget->setLockState(lockId, opened);
                
                QString cmdName = (result.command == Command::OpenSingle) ? "开锁" : "读取";
                m_debugConsole->logInfo(QString("%1操作 - 锁%2: %3")
                    .arg(cmdName).arg(lockId).arg(opened ? "打开" : "关闭"));
            } else {
                m_debugConsole->logError("单锁操作响应数据为空");
            }
            break;
            
        case Command::ReadAll:
        case Command::OpenAll:
        case Command::OpenMultiple:
        case Command::SimultaneousOpen:
            if (result.data.size() >= 2) {
                quint16 responseMask = (static_cast<quint8>(result.data[0]) << 8) | 
                                       static_cast<quint8>(result.data[1]);
                m_lockStatusWidget->setAllLockStates(responseMask);
                
                QString cmdName = Protocol485::commandToString(result.command);
                
                // 对于 OpenMultiple 和 SimultaneousOpen，对比请求和响应
                if (result.command == Command::OpenMultiple || result.command == Command::SimultaneousOpen) {
                    // 统计请求的锁数量
                    int requestedCount = 0;
                    QStringList requestedLocks;
                    for (int i = 0; i < 12; ++i) {
                        if (m_lastSentMask & (1 << i)) {
                            requestedCount++;
                            requestedLocks.append(QString::number(i + 1));
                        }
                    }
                    
                    // 统计实际开启的锁数量（响应掩码中的bit）
                    int actualCount = 0;
                    QStringList actualLocks;
                    for (int i = 0; i < 12; ++i) {
                        if (responseMask & (1 << i)) {
                            actualCount++;
                            actualLocks.append(QString::number(i + 1));
                        }
                    }
                    
                    // 检查是否匹配
                    if ((responseMask & m_lastSentMask) == m_lastSentMask) {
                        // 请求的锁都成功开启
                        m_debugConsole->logInfo(QString("%1操作完成 - 请求开启: %2个锁 [%3], 实际状态: %4个锁开启 [%5]")
                            .arg(cmdName)
                            .arg(requestedCount).arg(requestedLocks.join(", "))
                            .arg(actualCount).arg(actualLocks.join(", ")));
                        
                        if (actualCount > requestedCount) {
                            m_debugConsole->logWarning(QString("注意：设备返回的开启锁数量(%1)多于请求数量(%2)，可能其他锁也处于开启状态")
                                .arg(actualCount).arg(requestedCount));
                        }
                    } else {
                        // 有请求的锁未能开启
                        QStringList failedLocks;
                        for (int i = 0; i < 12; ++i) {
                            if ((m_lastSentMask & (1 << i)) && !(responseMask & (1 << i))) {
                                failedLocks.append(QString::number(i + 1));
                            }
                        }
                        
                        m_debugConsole->logError(QString("%1操作部分失败 - 请求: %2个 [%3], 成功: %4个 [%5], 失败: %6个 [%7]")
                            .arg(cmdName)
                            .arg(requestedCount).arg(requestedLocks.join(", "))
                            .arg(actualCount).arg(actualLocks.join(", "))
                            .arg(failedLocks.size()).arg(failedLocks.join(", ")));
                    }
                } else {
                    // OpenAll 和 ReadAll 不需要对比
                    int openCount = 0;
                    for (int i = 0; i < 12; ++i) {
                        if (responseMask & (1 << i)) openCount++;
                    }
                    
                    m_debugConsole->logInfo(QString("%1操作完成 - 开启数量: %2/12, 状态掩码: 0x%3")
                        .arg(cmdName).arg(openCount).arg(responseMask, 4, 16, QChar('0')));
                }
            } else {
                m_debugConsole->logError("批量操作响应数据长度不足");
            }
            break;
            
        case Command::ContinuousOutput:
        case Command::MultiOutput:
            if (result.data.size() >= 1) {
                bool success = static_cast<quint8>(result.data[0]) == 0x01;
                QString cmdName = Protocol485::commandToString(result.command);
                
                if (success) {
                    m_debugConsole->logInfo(QString("%1操作执行成功").arg(cmdName));
                } else {
                    m_debugConsole->logError(QString("%1操作失败，设备返回失败状态").arg(cmdName));
                    QMessageBox::warning(this, "操作失败", 
                        QString("%1操作未成功执行\n\n可能原因：\n"
                               "• 通道参数超出范围\n"
                               "• 设备内部状态异常\n"
                               "• 命令参数错误").arg(cmdName));
                }
            } else {
                m_debugConsole->logError("输出控制响应数据长度不足");
            }
            break;
            
        default:
            m_debugConsole->logError(QString("收到未知命令响应: 0x%1")
                .arg(static_cast<quint8>(result.command), 2, 16, QChar('0')));
            break;
    }
}

void MainWindow::onSerialError(const QString &error)
{
    m_debugConsole->logError(error);
}

void MainWindow::sendCommand(const QByteArray &data)
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先连接串口");
        return;
    }
    
    m_serialManager->sendData(data);
    m_debugConsole->logSend(data);
}

void MainWindow::onGetVersion()
{
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("获取版本号 - 设备地址:%1").arg(addr));
    
    QByteArray data = Protocol485::buildGetVersion(addr);
    sendCommand(data);
}

void MainWindow::onOpenSingleLock()
{
    QSpinBox *spin = m_tabWidget->currentWidget()->findChild<QSpinBox*>("singleLockSpin");
    if (!spin) {
        m_debugConsole->logError("无法找到锁号选择控件");
        return;
    }
    
    quint8 lockId = spin->value();
    
    // 参数验证
    if (lockId < 1 || lockId > 12) {
        QMessageBox::warning(this, "参数错误", 
            QString("锁号 %1 超出有效范围\n\n有效范围：1-12").arg(lockId));
        m_debugConsole->logError(QString("无效的锁号: %1").arg(lockId));
        return;
    }
    
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("准备开锁 - 设备地址:%1, 锁号:%2").arg(addr).arg(lockId));
    
    QByteArray data = Protocol485::buildOpenSingle(addr, lockId);
    sendCommand(data);
}

void MainWindow::onReadSingleLock()
{
    QSpinBox *spin = m_tabWidget->currentWidget()->findChild<QSpinBox*>("singleLockSpin");
    if (!spin) {
        m_debugConsole->logError("无法找到锁号选择控件");
        return;
    }
    
    quint8 lockId = spin->value();
    
    // 参数验证
    if (lockId < 1 || lockId > 12) {
        QMessageBox::warning(this, "参数错误", 
            QString("锁号 %1 超出有效范围\n\n有效范围：1-12").arg(lockId));
        m_debugConsole->logError(QString("无效的锁号: %1").arg(lockId));
        return;
    }
    
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("读取锁状态 - 设备地址:%1, 锁号:%2").arg(addr).arg(lockId));
    
    QByteArray data = Protocol485::buildReadSingle(addr, lockId);
    sendCommand(data);
}

void MainWindow::onReadAllLocks()
{
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("读取所有锁状态 - 设备地址:%1").arg(addr));
    
    QByteArray data = Protocol485::buildReadAll(addr);
    sendCommand(data);
}

void MainWindow::onOpenAllLocks()
{
    quint8 addr = m_addressSpin->value();
    
    // 二次确认
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认操作",
        QString("确定要开启设备地址 %1 的全部12个锁吗？\n\n"
               "此操作将同时触发所有锁的开锁动作。").arg(addr),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) {
        m_debugConsole->logInfo("用户取消了开全部锁操作");
        return;
    }
    
    m_debugConsole->logInfo(QString("开全部锁 - 设备地址:%1").arg(addr));
    
    QByteArray data = Protocol485::buildOpenAll(addr);
    sendCommand(data);
}

void MainWindow::onOpenMultipleLocks()
{
    quint16 mask = getLockMaskFromUI(m_tabWidget->currentWidget());
    if (mask == 0) {
        QMessageBox::information(this, "提示", 
            "请至少选择一个锁\n\n在多锁操作标签页中勾选需要操作的锁");
        m_debugConsole->logError("未选择任何锁");
        return;
    }
    
    // 统计选中的锁
    QStringList selectedLocks;
    for (int i = 1; i <= 12; ++i) {
        if (mask & (1 << (i - 1))) {
            selectedLocks.append(QString::number(i));
        }
    }
    
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("开多个锁(0x15) - 设备地址:%1, 选中锁: [%2], 掩码:0x%3")
        .arg(addr).arg(selectedLocks.join(", ")).arg(mask, 4, 16, QChar('0')));
    
    m_lastSentMask = mask; // 记录发送的掩码
    QByteArray data = Protocol485::buildOpenMultiple(addr, mask);
    sendCommand(data);
}

void MainWindow::onContinuousOutput()
{
    QSpinBox *spin = m_tabWidget->currentWidget()->findChild<QSpinBox*>("channelSpin");
    if (!spin) {
        m_debugConsole->logError("无法找到通道选择控件");
        return;
    }
    
    quint8 channel = spin->value();
    
    // 参数验证
    if (channel < 1 || channel > 12) {
        QMessageBox::warning(this, "参数错误", 
            QString("通道号 %1 超出有效范围\n\n有效范围：1-12").arg(channel));
        m_debugConsole->logError(QString("无效的通道号: %1").arg(channel));
        return;
    }
    
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("持续输出控制(0x16) - 设备地址:%1, 通道:%2, 状态:开").arg(addr).arg(channel));
    m_debugConsole->logWarning("此命令用于继电器、照明灯等持续通电设备，对电控锁使用可能造成损坏");
    
    QByteArray data = Protocol485::buildContinuousOutput(addr, channel, true);
    sendCommand(data);
}

void MainWindow::onSimultaneousOpen()
{
    quint16 mask = getLockMaskFromUI(m_tabWidget->currentWidget());
    if (mask == 0) {
        QMessageBox::information(this, "提示", 
            "请至少选择一个锁\n\n在多锁操作标签页中勾选需要操作的锁");
        m_debugConsole->logError("未选择任何锁");
        return;
    }
    
    // 统计选中的锁
    QStringList selectedLocks;
    for (int i = 1; i <= 12; ++i) {
        if (mask & (1 << (i - 1))) {
            selectedLocks.append(QString::number(i));
        }
    }
    
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("同时开锁(0x18) - 设备地址:%1, 选中锁: [%2], 掩码:0x%3")
        .arg(addr).arg(selectedLocks.join(", ")).arg(mask, 4, 16, QChar('0')));
    m_debugConsole->logInfo("说明：此命令用于同一门上安装多个锁的场景，所有锁同时动作");
    
    m_lastSentMask = mask; // 记录发送的掩码
    QByteArray data = Protocol485::buildSimultaneousOpen(addr, mask);
    sendCommand(data);
}

void MainWindow::onMultiOutput()
{
    quint16 mask = getLockMaskFromUI(m_tabWidget->currentWidget());
    if (mask == 0) {
        QMessageBox::information(this, "提示", 
            "请至少选择一个通道\n\n在持续输出标签页中勾选需要操作的通道");
        m_debugConsole->logError("未选择任何通道");
        return;
    }
    
    // 统计选中的通道
    QStringList selectedChannels;
    for (int i = 1; i <= 12; ++i) {
        if (mask & (1 << (i - 1))) {
            selectedChannels.append(QString::number(i));
        }
    }
    
    quint8 addr = m_addressSpin->value();
    m_debugConsole->logInfo(QString("多通道输出(0x1A) - 设备地址:%1, 选中通道: [%2], 掩码:0x%3")
        .arg(addr).arg(selectedChannels.join(", ")).arg(mask, 4, 16, QChar('0')));
    m_debugConsole->logWarning("此命令用于批量控制照明灯、继电器等持续通电设备");
    
    QByteArray data = Protocol485::buildMultiOutput(addr, mask);
    sendCommand(data);
}

quint16 MainWindow::getLockMaskFromUI(QWidget *parent)
{
    quint16 mask = 0;
    for (int i = 1; i <= 12; ++i) {
        QCheckBox *cb = parent->findChild<QCheckBox*>(QString("multiCheck_%1").arg(i));
        if (cb && cb->isChecked()) {
            mask |= (1 << (i - 1));
        }
    }
    return mask;
}

void MainWindow::onAutoConnect()
{
    if (m_autoConnecting) {
        // 停止自动连接
        m_autoConnecting = false;
        m_autoConnectTimer->stop();
        m_autoConnectButton->setText("自动连接");
        m_autoConnectButton->setEnabled(true);
        m_connectButton->setEnabled(true);
        m_portCombo->setEnabled(true);
        m_addressSpin->setEnabled(true);
        
        if (m_serialManager->isOpen()) {
            m_serialManager->closePort();
        }
        
        m_debugConsole->logInfo("自动连接已取消");
        return;
    }
    
    // 开始自动连接
    m_portsToTry = SerialManager::availablePortsSorted();
    
    if (m_portsToTry.isEmpty()) {
        QMessageBox::warning(this, "无可用串口", 
            "未检测到任何串口设备\n\n请检查：\n"
            "• USB转485设备是否已连接\n"
            "• 驱动程序是否正确安装");
        m_debugConsole->logError("自动连接失败：未发现可用串口");
        return;
    }
    
    m_autoConnecting = true;
    m_currentPortIndex = 0;
    m_currentAddress = 1;
    
    m_autoConnectButton->setText("停止扫描");
    m_connectButton->setEnabled(false);
    m_portCombo->setEnabled(false);
    m_addressSpin->setEnabled(false);
    
    m_debugConsole->logInfo(QString("====== 开始自动连接 ======"));
    m_debugConsole->logInfo(QString("发现 %1 个可用串口（已过滤虚拟串口），将尝试地址 1-15").arg(m_portsToTry.size()));
    m_debugConsole->logInfo(QString("串口列表: %1").arg(m_portsToTry.join(", ")));
    m_debugConsole->logInfo("提示：优先尝试 USB 转串口设备和 RS485 设备");
    
    // 开始第一次尝试
    performAutoConnect();
}

void MainWindow::performAutoConnect()
{
    if (!m_autoConnecting) {
        return;
    }
    
    // 检查是否已遍历所有串口
    if (m_currentPortIndex >= m_portsToTry.size()) {
        m_autoConnecting = false;
        m_autoConnectButton->setText("自动连接");
        m_autoConnectButton->setEnabled(true);
        m_connectButton->setEnabled(true);
        m_portCombo->setEnabled(true);
        m_addressSpin->setEnabled(true);
        
        m_debugConsole->logError("====== 自动连接失败 ======");
        m_debugConsole->logError("未找到响应的设备");
        
        QMessageBox::warning(this, "自动连接失败", 
            "未能找到响应的锁控板设备\n\n请检查：\n"
            "• 设备是否已上电\n"
            "• RS485接线是否正确（A-A, B-B）\n"
            "• 设备地址拨码是否在1-15范围内\n"
            "• USB转485设备是否正常工作");
        return;
    }
    
    QString currentPort = m_portsToTry[m_currentPortIndex];
    
    // 如果当前地址是1，说明要尝试新串口
    if (m_currentAddress == 1) {
        m_debugConsole->logInfo(QString("------ 尝试串口: %1 ------").arg(currentPort));
        
        // 关闭之前的串口
        if (m_serialManager->isOpen()) {
            m_serialManager->closePort();
        }
        
        // 打开新串口
        if (!m_serialManager->openPort(currentPort)) {
            m_debugConsole->logError(QString("无法打开 %1，跳过").arg(currentPort));
            m_currentPortIndex++;
            m_currentAddress = 1;
            m_autoConnectTimer->start(100);
            return;
        }
    }
    
    // 尝试当前地址
    m_debugConsole->logInfo(QString("  测试地址 %1...").arg(m_currentAddress));
    
    // 发送获取版本号命令
    QByteArray testCmd = Protocol485::buildGetVersion(m_currentAddress);
    m_serialManager->sendData(testCmd);
    
    // 等待500ms看是否有响应
    QTimer::singleShot(500, this, [this]() {
        if (!m_autoConnecting) {
            return;
        }
        
        // 如果没有找到设备，继续下一个地址
        m_currentAddress++;
        
        if (m_currentAddress > 15) {
            // 当前串口的所有地址都试过了，换下一个串口
            m_currentPortIndex++;
            m_currentAddress = 1;
        }
        
        performAutoConnect();
    });
}