#include "lockstatuswidget.h"
#include <QVBoxLayout>
#include <QGroupBox>

LockStatusWidget::LockStatusWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void LockStatusWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QGroupBox *groupBox = new QGroupBox("12路锁状态", this);
    QGridLayout *gridLayout = new QGridLayout();
    
    m_lockIndicators.resize(LOCK_COUNT);
    
    for (int i = 0; i < LOCK_COUNT; ++i) {
        int row = i / 4;
        int col = i % 4;
        
        QWidget *lockWidget = new QWidget();
        QVBoxLayout *lockLayout = new QVBoxLayout(lockWidget);
        lockLayout->setContentsMargins(5, 5, 5, 5);
        
        QLabel *numLabel = new QLabel(QString("锁 %1").arg(i + 1));
        numLabel->setAlignment(Qt::AlignCenter);
        
        QLabel *stateLabel = new QLabel("未知");
        stateLabel->setAlignment(Qt::AlignCenter);
        stateLabel->setMinimumSize(60, 30);
        stateLabel->setStyleSheet("background-color: #808080; color: white; "
                                  "border-radius: 5px; font-weight: bold;");
        
        lockLayout->addWidget(numLabel);
        lockLayout->addWidget(stateLabel);
        
        m_lockIndicators[i].numberLabel = numLabel;
        m_lockIndicators[i].stateLabel = stateLabel;
        m_lockIndicators[i].state = -1; // 未知
        
        gridLayout->addWidget(lockWidget, row, col);
    }
    
    groupBox->setLayout(gridLayout);
    mainLayout->addWidget(groupBox);
}

void LockStatusWidget::setLockState(int lockId, bool opened)
{
    if (lockId < 1 || lockId > LOCK_COUNT) return;
    
    int index = lockId - 1;
    m_lockIndicators[index].state = opened ? 1 : 0;
    
    m_lockIndicators[index].stateLabel->setText(getStateText(m_lockIndicators[index].state));
    m_lockIndicators[index].stateLabel->setStyleSheet(
        QString("background-color: %1; color: white; border-radius: 5px; font-weight: bold;")
        .arg(getStateColor(m_lockIndicators[index].state))
    );
}

void LockStatusWidget::setAllLockStates(quint16 lockMask)
{
    for (int i = 0; i < LOCK_COUNT; ++i) {
        bool opened = (lockMask & (1 << i)) != 0;
        setLockState(i + 1, opened);
    }
}

void LockStatusWidget::resetAllStates()
{
    for (int i = 0; i < LOCK_COUNT; ++i) {
        m_lockIndicators[i].state = -1;
        m_lockIndicators[i].stateLabel->setText("未知");
        m_lockIndicators[i].stateLabel->setStyleSheet(
            "background-color: #808080; color: white; border-radius: 5px; font-weight: bold;"
        );
    }
}

QString LockStatusWidget::getStateColor(int state) const
{
    switch (state) {
        case 0: return "#FF4444"; // 红色 - 关闭
        case 1: return "#44FF44"; // 绿色 - 打开
        default: return "#808080"; // 灰色 - 未知
    }
}

QString LockStatusWidget::getStateText(int state) const
{
    switch (state) {
        case 0: return "关闭";
        case 1: return "打开";
        default: return "未知";
    }
}