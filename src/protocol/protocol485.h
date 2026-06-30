#ifndef PROTOCOL485_H
#define PROTOCOL485_H

#include <QObject>
#include <QByteArray>

// 协议命令定义
enum class Command : quint8 {
    GetVersion = 0x01,      // 获取单片机版本号
    OpenSingle = 0x10,      // 开单个锁
    ReadSingle = 0x11,      // 读取单个锁状态
    ReadAll = 0x12,         // 读取所有锁状态
    OpenAll = 0x14,         // 开全部锁
    OpenMultiple = 0x15,    // 开多个锁
    ContinuousOutput = 0x16,// 锁通道持续输出
    SimultaneousOpen = 0x18,// 同时开多个锁
    MultiOutput = 0x1A      // 多个通道输出控制
};

// 锁状态定义
enum class LockState : quint8 {
    Unknown = 0xFF,
    Closed = 0x00,
    Opened = 0x01
};

// 协议解析结果
struct ParseResult {
    bool success;
    quint8 address;
    Command command;
    QByteArray data;
    QString errorMsg;
};

class Protocol485 : public QObject
{
    Q_OBJECT

public:
    explicit Protocol485(QObject *parent = nullptr);

    // 构建发送数据帧
    static QByteArray buildGetVersion(quint8 address);
    static QByteArray buildOpenSingle(quint8 address, quint8 lockId);
    static QByteArray buildReadSingle(quint8 address, quint8 lockId);
    static QByteArray buildReadAll(quint8 address);
    static QByteArray buildOpenAll(quint8 address);
    static QByteArray buildOpenMultiple(quint8 address, quint16 lockMask);
    static QByteArray buildContinuousOutput(quint8 address, quint8 channel, bool on);
    static QByteArray buildSimultaneousOpen(quint8 address, quint16 lockMask);
    static QByteArray buildMultiOutput(quint8 address, quint16 channelMask);

    // 解析接收数据帧
    static ParseResult parseResponse(const QByteArray &data);
    
    // 辅助函数
    static QString commandToString(Command cmd);
    static quint8 calculateChecksum(const QByteArray &data, int startPos, int length);

private:
    static QByteArray buildFrame(quint8 address, Command cmd, const QByteArray &payload);
};

#endif // PROTOCOL485_H