#ifndef LOCKSTATUSWIDGET_H
#define LOCKSTATUSWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QGridLayout>

class LockStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LockStatusWidget(QWidget *parent = nullptr);

    // 设置单个锁的状态
    void setLockState(int lockId, bool opened);
    
    // 设置所有锁的状态（通过16位掩码）
    void setAllLockStates(quint16 lockMask);
    
    // 重置所有锁为未知状态
    void resetAllStates();

private:
    void setupUI();
    QString getStateColor(int state) const;
    QString getStateText(int state) const;

    struct LockIndicator {
        QLabel *numberLabel;
        QLabel *stateLabel;
        int state; // 0=关闭, 1=打开, -1=未知
    };

    QVector<LockIndicator> m_lockIndicators;
    static const int LOCK_COUNT = 12;
};

#endif // LOCKSTATUSWIDGET_H