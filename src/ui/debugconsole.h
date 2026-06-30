#ifndef DEBUGCONSOLE_H
#define DEBUGCONSOLE_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>

class DebugConsole : public QWidget
{
    Q_OBJECT

public:
    explicit DebugConsole(QWidget *parent = nullptr);

    // 添加发送数据日志
    void logSend(const QByteArray &data);
    
    // 添加接收数据日志
    void logReceive(const QByteArray &data);
    
    // 添加错误日志
    void logError(const QString &error);
    
    // 添加警告日志
    void logWarning(const QString &warning);
    
    // 添加信息日志
    void logInfo(const QString &info);
    
    // 清空日志
    void clear();

private:
    void setupUI();
    QString formatHexData(const QByteArray &data);
    QString getCurrentTimestamp();

    QTextEdit *m_logText;
    QPushButton *m_clearButton;
    QCheckBox *m_autoScrollCheck;
    QCheckBox *m_timestampCheck;
};

#endif // DEBUGCONSOLE_H