# Linux 平台安装指南

## 系统要求

- Linux 发行版：Ubuntu 20.04+、Fedora 35+、Arch Linux 或其他主流发行版
- Qt 5.15+ 或 Qt 6.x
- 支持的串口芯片：CH340、CP210x、FT232、PL2303 等

## 快速安装

### 使用自动化脚本（推荐）

```bash
# 进入项目目录
cd lockBoard

# 运行安装脚本（需要 root 权限）
sudo bash scripts/setup-linux.sh
```

脚本会自动完成：
- 检测 Linux 发行版
- 安装 Qt 开发依赖
- 配置串口权限
- 安装 udev 规则

## 手动安装步骤

### 1. 安装 Qt 依赖

#### Ubuntu / Debian 系

```bash
sudo apt update
sudo apt install -y \
    qt5-qmake \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5serialport5-dev \
    build-essential
```

#### Fedora / RHEL / CentOS 系

```bash
# Fedora
sudo dnf install -y \
    qt5-qtbase-devel \
    qt5-qtserialport-devel \
    gcc-c++ \
    make

# RHEL/CentOS (需要 EPEL 仓库)
sudo yum install epel-release -y
sudo yum install -y \
    qt5-qtbase-devel \
    qt5-qtserialport-devel \
    gcc-c++ \
    make
```

#### Arch Linux / Manjaro

```bash
sudo pacman -S --noconfirm \
    qt5-base \
    qt5-serialport \
    base-devel
```

#### openSUSE

```bash
sudo zypper install -y \
    libqt5-qtbase-devel \
    libqt5-qtserialport-devel \
    gcc-c++ \
    make
```

### 2. 编译项目

```bash
# 进入项目目录
cd lockBoard

# 生成 Makefile
qmake LockBoardTester.pro

# 编译（使用多核加速）
make -j$(nproc)

# 可执行文件生成在当前目录或 release/ 目录
```

### 3. 配置串口权限

Linux 下访问串口设备需要适当的用户权限。

#### 方法一：添加用户到 dialout 组（推荐）

```bash
# 将当前用户添加到 dialout 组
sudo usermod -aG dialout $USER

# Arch Linux 使用 uucp 组
# sudo usermod -aG uucp $USER

# 注销后重新登录生效，或使用以下命令立即生效
newgrp dialout
```

#### 方法二：安装 udev 规则（适用于 USB 转串口）

```bash
# 复制 udev 规则文件
sudo cp scripts/99-lockboard.rules /etc/udev/rules.d/

# 重新加载 udev 规则
sudo udevadm control --reload-rules
sudo udevadm trigger

# 重新插拔 USB 转串口设备
```

### 4. 运行程序

```bash
# 直接运行
./LockBoardTester

# 或者从 release 目录运行
./release/LockBoardTester
```

## 安装到系统（可选）

将程序安装到系统路径，方便从应用菜单启动：

```bash
# 安装到 /usr/local（默认）
sudo make install

# 自定义安装路径
qmake PREFIX=/usr LockBoardTester.pro
make
sudo make install
```

安装后可以：
- 从应用菜单找到"锁控板测试工具"
- 或在终端输入 `LockBoardTester` 直接启动

## 常见问题

### Q: 无法打开串口，提示权限不足？

**A**: 当前用户没有串口访问权限。

**解决方法**：
```bash
# 1. 检查串口设备权限
ls -l /dev/ttyUSB0
# 输出类似：crw-rw---- 1 root dialout 188, 0 6月 30 10:00 /dev/ttyUSB0

# 2. 确认自己在 dialout 组中
groups
# 如果没有 dialout，执行：
sudo usermod -aG dialout $USER

# 3. 注销重新登录，或执行
newgrp dialout

# 4. 如果还不行，安装 udev 规则
sudo cp scripts/99-lockboard.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Q: 编译时提示找不到 Qt 模块？

**A**: Qt 开发包未正确安装。

**解决方法**：
```bash
# 检查 Qt 是否安装
qmake --version

# 如果未安装，根据发行版安装对应的包（见上方"安装 Qt 依赖"）

# 确认 Qt5SerialPort 模块存在
# Ubuntu/Debian:
dpkg -l | grep libqt5serialport

# Fedora/RHEL:
rpm -qa | grep qt5-qtserialport

# Arch:
pacman -Q qt5-serialport
```

### Q: 插入 USB 转串口后找不到 /dev/ttyUSB* 设备？

**A**: 内核驱动未加载或硬件不兼容。

**解决方法**：
```bash
# 1. 插入设备后查看内核日志
dmesg | tail -20

# 2. 检查设备是否被识别
lsusb
# 找到对应的 USB 转串口设备

# 3. 常见芯片驱动模块
# CH340/CH341:
sudo modprobe ch341

# CP210x:
sudo modprobe cp210x

# FT232:
sudo modprobe ftdi_sio

# PL2303:
sudo modprobe pl2303

# 4. 查看已加载的串口设备
ls /dev/tty* | grep USB
```

### Q: Wayland 下程序界面显示异常？

**A**: Qt 5 在某些 Wayland 环境下可能有兼容性问题。

**解决方法**：
```bash
# 强制使用 X11 后端运行
QT_QPA_PLATFORM=xcb ./LockBoardTester

# 或设置环境变量
export QT_QPA_PLATFORM=xcb
./LockBoardTester
```

### Q: 如何卸载？

**A**: 
```bash
# 如果使用 make install 安装
sudo make uninstall

# 手动删除文件
sudo rm /usr/local/bin/LockBoardTester
sudo rm /usr/local/share/applications/lockboard-tester.desktop
sudo rm /usr/local/share/icons/hicolor/48x48/apps/lockboard.png

# 删除 udev 规则
sudo rm /etc/udev/rules.d/99-lockboard.rules
sudo udevadm control --reload-rules
```

## 串口设备命名

不同发行版的串口设备命名可能不同：

| 设备类型 | Ubuntu/Debian | Arch Linux | Fedora |
|---------|---------------|------------|--------|
| USB转串口 | /dev/ttyUSB0 | /dev/ttyUSB0 | /dev/ttyUSB0 |
| 原生串口 | /dev/ttyS0 | /dev/ttyS0 | /dev/ttyS0 |
| USB ACM | /dev/ttyACM0 | /dev/ttyACM0 | /dev/ttyACM0 |

## 性能优化建议

### 1. 实时优先级（可选）

如果遇到串口通讯延迟问题：

```bash
# 以实时优先级运行（需要 root 权限）
sudo chrt -r 50 ./LockBoardTester
```

### 2. 禁用节能模式

```bash
# 禁用 USB 自动挂起（临时）
echo -1 | sudo tee /sys/bus/usb/devices/*/power/autosuspend_delay_ms

# 永久禁用，添加内核参数：
# 编辑 /etc/default/grub，在 GRUB_CMDLINE_LINUX 中添加：
# usbcore.autosuspend=-1
```

## 开发环境设置

如需在 Linux 下进行二次开发：

```bash
# 安装 Qt Creator
# Ubuntu/Debian:
sudo apt install qtcreator

# Fedora:
sudo dnf install qt-creator

# Arch:
sudo pacman -S qtcreator

# 打开项目
qtcreator LockBoardTester.pro
```

## 技术支持

遇到问题？

1. 检查 [常见问题](#常见问题) 章节
2. 查看程序调试控制台的错误信息
3. 确认硬件连接和驱动正常
4. 提交 Issue 时附带：
   - 发行版和版本号（`cat /etc/os-release`）
   - Qt 版本（`qmake --version`）
   - 错误日志和调试信息

## 相关资源

- Qt 官方文档：https://doc.qt.io/qt-5/qserialport.html
- Linux Serial Port Programming：https://tldp.org/HOWTO/Serial-Programming-HOWTO/
- udev 规则编写指南：https://wiki.archlinux.org/title/Udev