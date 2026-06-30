#include "debugconsole.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QScrollBar>

DebugConsole::DebugConsole(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void DebugConsole::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 日志文本框
    m_logText = new QTextEdit();
    m_logText->setReadOnly(true);
    m_logText->setFont(QFont("Consolas", 9));
    m_logText->setMinimumHeight(200);
    
    // 控制按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    m_clearButton = new QPushButton("清空日志");
    m_autoScrollCheck = new QCheckBox("自动滚动");
    m_autoScrollCheck->setChecked(true);
    m_timestampCheck = new QCheckBox("显示时间戳");
    m_timestampCheck->setChecked(true);
    
    controlLayout->addWidget(m_clearButton);
    controlLayout->addWidget(m_autoScrollCheck);
    controlLayout->addWidget(m_timestampCheck);
    controlLayout->addStretch();
    
    mainLayout->addWidget(m_logText);
    mainLayout->addLayout(controlLayout);
    
    // 连接信号
    connect(m_clearButton, &QPushButton::clicked, this, &DebugConsole::clear);
}

void DebugConsole::logSend(const QByteArray &data)
{
    QString timestamp = m_timestampCheck->isChecked() ? getCurrentTimestamp() + " " : "";
    QString logLine = QString("<font color='blue'>%1[发送] %2</font>")
                          .arg(timestamp)
                          .arg(formatHexData(data));
    
    m_logText->append(logLine);
    
    if (m_autoScrollCheck->isChecked()) {
        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
    }
}

void DebugConsole::logReceive(const QByteArray &data)
{
    QString timestamp = m_timestampCheck->isChecked() ? getCurrentTimestamp() + " " : "";
    QString logLine = QString("<font color='green'>%1[接收] %2</font>")
                          .arg(timestamp)
                          .arg(formatHexData(data));
    
    m_logText->append(logLine);
    
    if (m_autoScrollCheck->isChecked()) {
        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
    }
}

void DebugConsole::logError(const QString &error)
{
    QString timestamp = m_timestampCheck->isChecked() ? getCurrentTimestamp() + " " : "";
    QString logLine = QString("<font color='red'>%1[错误] %2</font>")
                          .arg(timestamp)
                          .arg(error);
    
    m_logText->append(logLine);
    
    if (m_autoScrollCheck->isChecked()) {
        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
    }
}

void DebugConsole::logWarning(const QString &warning)
{
    QString timestamp = m_timestampCheck->isChecked() ? getCurrentTimestamp() + " " : "";
    QString logLine = QString("<font color='orange'>%1[警告] %2</font>")
                          .arg(timestamp)
                          .arg(warning);
    
    m_logText->append(logLine);
    
    if (m_autoScrollCheck->isChecked()) {
        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
    }
}

void DebugConsole::logInfo(const QString &info)
{
    QString timestamp = m_timestampCheck->isChecked() ? getCurrentTimestamp() + " " : "";
    QString logLine = QString("<font color='black'>%1[信息] %2</font>")
                          .arg(timestamp)
                          .arg(info);
    
    m_logText->append(logLine);
    
    if (m_autoScrollCheck->isChecked()) {
        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
    }
}

void DebugConsole::clear()
{
    m_logText->clear();
}

QString DebugConsole::formatHexData(const QByteArray &data)
{
    QString hex;
    for (int i = 0; i < data.size(); ++i) {
        hex += QString("%1 ").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return hex.trimmed();
}

QString DebugConsole::getCurrentTimestamp()
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
}