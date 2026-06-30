#include "protocol485.h"

Protocol485::Protocol485(QObject *parent) : QObject(parent)
{
}

QByteArray Protocol485::buildFrame(quint8 address, Command cmd, const QByteArray &payload)
{
    QByteArray frame;
    frame.append(static_cast<char>(0xDC)); // 包头1
    frame.append(static_cast<char>(0x02)); // 包头2
    frame.append(static_cast<char>(address)); // 地址
    frame.append(static_cast<char>(cmd)); // 指令
    frame.append(static_cast<char>(payload.size())); // 数据长度
    
    if (!payload.isEmpty()) {
        frame.append(payload); // 数据段
    }
    
    // 计算校验：地址+指令+长度+数据段
    quint8 checksum = address + static_cast<quint8>(cmd) + payload.size();
    for (int i = 0; i < payload.size(); ++i) {
        checksum += static_cast<quint8>(payload[i]);
    }
    frame.append(static_cast<char>(checksum % 256));
    
    return frame;
}

QByteArray Protocol485::buildGetVersion(quint8 address)
{
    return buildFrame(address, Command::GetVersion, QByteArray());
}

QByteArray Protocol485::buildOpenSingle(quint8 address, quint8 lockId)
{
    QByteArray payload;
    payload.append(static_cast<char>(lockId));
    return buildFrame(address, Command::OpenSingle, payload);
}

QByteArray Protocol485::buildReadSingle(quint8 address, quint8 lockId)
{
    QByteArray payload;
    payload.append(static_cast<char>(lockId));
    return buildFrame(address, Command::ReadSingle, payload);
}

QByteArray Protocol485::buildReadAll(quint8 address)
{
    return buildFrame(address, Command::ReadAll, QByteArray());
}

QByteArray Protocol485::buildOpenAll(quint8 address)
{
    return buildFrame(address, Command::OpenAll, QByteArray());
}

QByteArray Protocol485::buildOpenMultiple(quint8 address, quint16 lockMask)
{
    QByteArray payload;
    payload.append(static_cast<char>((lockMask >> 8) & 0xFF)); // 高字节
    payload.append(static_cast<char>(lockMask & 0xFF));        // 低字节
    return buildFrame(address, Command::OpenMultiple, payload);
}

QByteArray Protocol485::buildContinuousOutput(quint8 address, quint8 channel, bool on)
{
    QByteArray payload;
    payload.append(static_cast<char>(channel));
    payload.append(static_cast<char>(on ? 0x01 : 0x00));
    return buildFrame(address, Command::ContinuousOutput, payload);
}

QByteArray Protocol485::buildSimultaneousOpen(quint8 address, quint16 lockMask)
{
    QByteArray payload;
    payload.append(static_cast<char>((lockMask >> 8) & 0xFF)); // 高字节
    payload.append(static_cast<char>(lockMask & 0xFF));        // 低字节
    return buildFrame(address, Command::SimultaneousOpen, payload);
}

QByteArray Protocol485::buildMultiOutput(quint8 address, quint16 channelMask)
{
    QByteArray payload;
    payload.append(static_cast<char>((channelMask >> 8) & 0xFF)); // 高字节
    payload.append(static_cast<char>(channelMask & 0xFF));        // 低字节
    return buildFrame(address, Command::MultiOutput, payload);
}

ParseResult Protocol485::parseResponse(const QByteArray &data)
{
    ParseResult result;
    result.success = false;
    
    // 最小长度检查：包头(2) + 地址(1) + 指令(1) + 长度(1) + 校验(1) = 6
    if (data.size() < 6) {
        result.errorMsg = "数据长度不足";
        return result;
    }
    
    // 检查包头
    if (static_cast<quint8>(data[0]) != 0xDC || 
        static_cast<quint8>(data[1]) != 0xFE) {
        result.errorMsg = "包头错误";
        return result;
    }
    
    result.address = static_cast<quint8>(data[2]);
    result.command = static_cast<Command>(static_cast<quint8>(data[3]));
    quint8 dataLen = static_cast<quint8>(data[4]);
    
    // 长度验证
    if (data.size() != 6 + dataLen) {
        result.errorMsg = QString("数据长度不匹配，期望%1，实际%2").arg(6 + dataLen).arg(data.size());
        return result;
    }
    
    // 提取数据段
    if (dataLen > 0) {
        result.data = data.mid(5, dataLen);
    }
    
    // 校验验证
    quint8 receivedChecksum = static_cast<quint8>(data[5 + dataLen]);
    quint8 calculatedChecksum = result.address + static_cast<quint8>(result.command) + dataLen;
    for (int i = 0; i < dataLen; ++i) {
        calculatedChecksum += static_cast<quint8>(result.data[i]);
    }
    calculatedChecksum = calculatedChecksum % 256;
    
    if (receivedChecksum != calculatedChecksum) {
        result.errorMsg = QString("校验错误，接收%1，计算%2")
            .arg(receivedChecksum, 2, 16, QChar('0'))
            .arg(calculatedChecksum, 2, 16, QChar('0'));
        return result;
    }
    
    result.success = true;
    return result;
}

QString Protocol485::commandToString(Command cmd)
{
    switch (cmd) {
        case Command::GetVersion: return "获取版本";
        case Command::OpenSingle: return "开单锁";
        case Command::ReadSingle: return "读单锁";
        case Command::ReadAll: return "读全部";
        case Command::OpenAll: return "开全部";
        case Command::OpenMultiple: return "开多锁";
        case Command::ContinuousOutput: return "持续输出";
        case Command::SimultaneousOpen: return "同时开锁";
        case Command::MultiOutput: return "多通道输出";
        default: return "未知命令";
    }
}

quint8 Protocol485::calculateChecksum(const QByteArray &data, int startPos, int length)
{
    quint8 sum = 0;
    for (int i = startPos; i < startPos + length && i < data.size(); ++i) {
        sum += static_cast<quint8>(data[i]);
    }
    return sum % 256;
}