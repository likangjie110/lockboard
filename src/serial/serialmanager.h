#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QByteArray>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    // 串口操作
    bool openPort(const QString &portName);
    void closePort();
    bool isOpen() const;
    QString currentPortName() const;
    
    // 数据发送
    bool sendData(const QByteArray &data);
    
    // 超时设置
    void setResponseTimeout(int milliseconds);
    int getResponseTimeout() const;
    
    // 获取可用串口列表
    static QStringList availablePorts();
    
    // 获取带优先级排序的串口列表（用于自动连接）
    static QStringList availablePortsSorted();

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void portOpened(const QString &portName);
    void portClosed();
    void responseTimeout();
    void portDisconnected();

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
    void handleTimeout();

private:
    QSerialPort *m_serialPort;
    QByteArray m_receiveBuffer;
    QTimer *m_timeoutTimer;
    int m_timeoutMs;
};

#endif // SERIALMANAGER_H