#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include "serialmanager.h"
#include "protocol485.h"
#include "lockstatuswidget.h"
#include "debugconsole.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 串口连接
    void onRefreshPorts();
    void onConnectToggle();
    void onAutoConnect();
    
    // 协议处理
    void onDataReceived(const QByteArray &data);
    void onSerialError(const QString &error);
    
    // 功能操作
    void onGetVersion();
    void onOpenSingleLock();
    void onReadSingleLock();
    void onReadAllLocks();
    void onOpenAllLocks();
    void onOpenMultipleLocks();
    void onContinuousOutput();
    void onSimultaneousOpen();
    void onMultiOutput();

private:
    void setupUI();
    void setupConnections();
    void sendCommand(const QByteArray &data);
    quint16 getLockMaskFromUI(QWidget *parent);
    void performAutoConnect();

    Ui::MainWindow *ui;
    
    SerialManager *m_serialManager;
    LockStatusWidget *m_lockStatusWidget;
    DebugConsole *m_debugConsole;
    
    // 自动连接状态
    bool m_autoConnecting;
    QStringList m_portsToTry;
    int m_currentPortIndex;
    int m_currentAddress;
    QTimer *m_autoConnectTimer;
    
    // 记录最后发送的命令掩码（用于对比响应）
    quint16 m_lastSentMask;
    
    // UI组件
    QComboBox *m_portCombo;
    QPushButton *m_refreshButton;
    QPushButton *m_connectButton;
    QPushButton *m_autoConnectButton;
    QSpinBox *m_addressSpin;
    QTabWidget *m_tabWidget;
};

#endif // MAINWINDOW_H