QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 版本信息
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/protocol/protocol485.cpp \
    src/serial/serialmanager.cpp \
    src/ui/lockstatuswidget.cpp \
    src/ui/debugconsole.cpp

HEADERS += \
    src/mainwindow.h \
    src/protocol/protocol485.h \
    src/serial/serialmanager.h \
    src/ui/lockstatuswidget.h \
    src/ui/debugconsole.h

FORMS += \
    src/ui/mainwindow.ui

INCLUDEPATH += \
    src \
    src/protocol \
    src/serial \
    src/ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 编译优化选项
CONFIG(release, debug|release) {
    # Release 模式优化
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += -O3
    
    # 去除调试符号
    QMAKE_LFLAGS_RELEASE += -s
    
    # Windows 平台禁用控制台窗口
    win32: CONFIG += windows
}

# Windows 平台配置
win32 {
    # Windows 可执行文件保持在 release 目录
    DESTDIR = release
    
    # 应用程序图标（需要创建 resources/app.rc）
    RC_ICONS = resources/lockboard.ico
    
    # 版本信息
    QMAKE_TARGET_COMPANY = "LockBoard Project"
    QMAKE_TARGET_PRODUCT = "LockBoard Tester"
    QMAKE_TARGET_DESCRIPTION = "RS485 Lock Control Board Communication Tester"
    QMAKE_TARGET_COPYRIGHT = "Copyright 2026"
}

# Linux 平台配置
unix:!macx {
    # Linux 安装路径
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    
    target.path = $$PREFIX/bin
    
    # 桌面文件安装
    desktop.files = lockboard-tester.desktop
    desktop.path = $$PREFIX/share/applications
    
    # 图标安装
    icon.files = resources/lockboard.png
    icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
    
    INSTALLS += target desktop icon
}