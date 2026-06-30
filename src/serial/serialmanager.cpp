#include "serialmanager.h"
#include <QDebug>
#include <algorithm>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_timeoutTimer(new QTimer(this))
    , m_timeoutMs(2000) // 默认2秒超时
{
    connect(m_serialPort, &QSerialPort::readyRead, 
            this, &SerialManager::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, 
            this, &SerialManager::handleError);
    
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &SerialManager::handleTimeout);
}

SerialManager::~SerialManager()
{
    closePort();
}

bool SerialManager::openPort(const QString &portName)
{
    if (m_serialPort->isOpen()) {
        closePort();
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_receiveBuffer.clear();
        emit portOpened(portName);
        return true;
    } else {
        emit errorOccurred(QString("打开串口失败: %1").arg(m_serialPort->errorString()));
        return false;
    }
}

void SerialManager::closePort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        m_receiveBuffer.clear();
        emit portClosed();
    }
}

bool SerialManager::isOpen() const
{
    return m_serialPort && m_serialPort->isOpen();
}

QString SerialManager::currentPortName() const
{
    return m_serialPort->portName();
}

bool SerialManager::sendData(const QByteArray &data)
{
    if (!m_serialPort->isOpen()) {
        emit errorOccurred("串口未打开");
        return false;
    }
    
    if (data.isEmpty()) {
        emit errorOccurred("发送数据为空");
        return false;
    }

    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred(QString("发送失败: %1").arg(m_serialPort->errorString()));
        return false;
    }
    
    if (bytesWritten != data.size()) {
        emit errorOccurred(QString("数据未完全发送: 期望%1字节，实际%2字节")
                          .arg(data.size()).arg(bytesWritten));
        return false;
    }
    
    m_serialPort->flush();
    
    // 启动超时定时器
    m_timeoutTimer->start(m_timeoutMs);
    
    return true;
}

QStringList SerialManager::availablePorts()
{
    QStringList portList;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        // 过滤掉虚拟串口和不太可能是RS485设备的串口
        QString description = info.description().toLower();
        QString manufacturer = info.manufacturer().toLower();
        
        // 跳过蓝牙串口
        if (description.contains("bluetooth") || manufacturer.contains("bluetooth")) {
            continue;
        }
        
        // 跳过明确标记为虚拟的串口
        if (description.contains("virtual") || description.contains("loopback")) {
            continue;
        }
        
        // 跳过 Windows 内部串口
        if (description.contains("communications port") && info.systemLocation().isEmpty()) {
            continue;
        }
        
        portList << info.portName();
    }
    return portList;
}

QStringList SerialManager::availablePortsSorted()
{
    struct PortInfo {
        QString name;
        int priority;
    };
    
    QList<PortInfo> ports;
    const auto infos = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &info : infos) {
        QString description = info.description().toLower();
        QString manufacturer = info.manufacturer().toLower();
        int priority = 50; // 默认优先级
        
        // 跳过蓝牙串口
        if (description.contains("bluetooth") || manufacturer.contains("bluetooth")) {
            continue;
        }
        
        // 跳过明确标记为虚拟的串口
        if (description.contains("virtual") || description.contains("loopback")) {
            continue;
        }
        
        // 跳过 Windows 内部串口
        if (description.contains("communications port") && info.systemLocation().isEmpty()) {
            continue;
        }
        
        // USB转串口设备（高优先级）
        if (description.contains("usb") || manufacturer.contains("usb") ||
            description.contains("ch340") || description.contains("ch341") ||
            description.contains("cp210") || description.contains("ft232") ||
            description.contains("pl2303") || manufacturer.contains("ftdi") ||
            manufacturer.contains("prolific") || manufacturer.contains("silicon labs")) {
            priority = 10; // 高优先级
        }
        
        // RS485相关设备（最高优先级）
        if (description.contains("485") || description.contains("rs485") ||
            description.contains("rs-485")) {
            priority = 1; // 最高优先级
        }
        
        // 有厂商信息的设备优先级提高
        if (!manufacturer.isEmpty() && manufacturer != "unknown") {
            priority -= 5;
        }
        
        ports.append({info.portName(), priority});
    }
    
    // 按优先级排序（优先级数字越小越靠前）
    std::sort(ports.begin(), ports.end(), [](const PortInfo &a, const PortInfo &b) {
        if (a.priority != b.priority) {
            return a.priority < b.priority;
        }
        // 优先级相同时按串口号排序
        return a.name < b.name;
    });
    
    QStringList result;
    for (const PortInfo &port : ports) {
        result << port.name;
    }
    
    return result;
}

void SerialManager::handleReadyRead()
{
    // 停止超时定时器，因为收到了数据
    m_timeoutTimer->stop();
    
    QByteArray newData = m_serialPort->readAll();
    m_receiveBuffer.append(newData);
    
    // 查找完整的数据帧 (0xDC 0xFE 开头)
    while (m_receiveBuffer.size() >= 6) { // 最小帧长度
        int headerIndex = -1;
        
        // 寻找包头
        for (int i = 0; i <= m_receiveBuffer.size() - 2; ++i) {
            if (static_cast<quint8>(m_receiveBuffer[i]) == 0xDC && 
                static_cast<quint8>(m_receiveBuffer[i + 1]) == 0xFE) {
                headerIndex = i;
                break;
            }
        }
        
        if (headerIndex == -1) {
            // 没有找到包头，记录无效数据后清空缓冲区
            if (m_receiveBuffer.size() > 0) {
                emit errorOccurred(QString("接收到无效数据包头，已丢弃%1字节").arg(m_receiveBuffer.size()));
            }
            m_receiveBuffer.clear();
            break;
        }
        
        // 丢弃包头之前的数据
        if (headerIndex > 0) {
            emit errorOccurred(QString("丢弃包头前的%1字节无效数据").arg(headerIndex));
            m_receiveBuffer.remove(0, headerIndex);
        }
        
        // 检查是否有足够的数据来确定帧长度
        if (m_receiveBuffer.size() < 5) {
            break; // 等待更多数据
        }
        
        // 获取数据长度
        quint8 dataLen = static_cast<quint8>(m_receiveBuffer[4]);
        int totalFrameLen = 6 + dataLen; // 包头(2)+地址(1)+指令(1)+长度(1)+数据(dataLen)+校验(1)
        
        if (m_receiveBuffer.size() < totalFrameLen) {
            break; // 等待更多数据
        }
        
        // 提取完整帧
        QByteArray frame = m_receiveBuffer.left(totalFrameLen);
        m_receiveBuffer.remove(0, totalFrameLen);
        
        // 发送完整帧
        emit dataReceived(frame);
    }
}

void SerialManager::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        return;
    }
    
    QString errorMsg;
    switch (error) {
        case QSerialPort::DeviceNotFoundError:
            errorMsg = "设备未找到";
            break;
        case QSerialPort::PermissionError:
            errorMsg = "权限错误，设备可能被其他程序占用";
            break;
        case QSerialPort::OpenError:
            errorMsg = "无法打开设备";
            break;
        case QSerialPort::WriteError:
            errorMsg = "写入数据失败";
            break;
        case QSerialPort::ReadError:
            errorMsg = "读取数据失败";
            break;
        case QSerialPort::ResourceError:
            errorMsg = "设备资源不可用，可能已断开连接";
            emit portDisconnected();
            break;
        case QSerialPort::UnsupportedOperationError:
            errorMsg = "不支持的操作";
            break;
        case QSerialPort::NotOpenError:
            errorMsg = "设备未打开";
            break;
        default:
            errorMsg = m_serialPort->errorString();
            break;
    }
    
    emit errorOccurred(errorMsg);
}

void SerialManager::handleTimeout()
{
    emit errorOccurred(QString("响应超时：%1毫秒内未收到设备响应").arg(m_timeoutMs));
    emit responseTimeout();
}

void SerialManager::setResponseTimeout(int milliseconds)
{
    if (milliseconds > 0) {
        m_timeoutMs = milliseconds;
    }
}

int SerialManager::getResponseTimeout() const
{
    return m_timeoutMs;
}