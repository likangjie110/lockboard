# Linux 系统故障排除指南

本文档提供 LockBoardTester 在 Linux 系统上常见问题的详细解决方案。

## 目录

- [串口权限问题](#串口权限问题)
- [设备未识别问题](#设备未识别问题)
- [Qt 版本不匹配问题](#qt-版本不匹配问题)
- [中文字体显示问题](#中文字体显示问题)
- [各发行版特定问题](#各发行版特定问题)

---

## 串口权限问题

### 现象描述

启动程序后无法打开串口，出现以下错误之一：
- 程序提示：`Permission denied` 或 `访问被拒绝`
- 调试控制台显示：`Cannot open /dev/ttyUSB0: Permission denied`
- 串口列表为空或串口显示但无法连接
- 连接按钮点击后无响应或立即断开

### 原因分析

Linux 系统中串口设备（`/dev/ttyUSB*`, `/dev/ttyACM*`, `/dev/ttyS*`）默认属于 `dialout` 用户组，普通用户没有直接访问权限。这是 Linux 系统的安全机制。

设备权限通常是：
```bash
crw-rw---- 1 root dialout 188, 0 Jun 30 10:00 /dev/ttyUSB0
```
其中 `rw-` 表示 dialout 组有读写权限，其他用户无权限。

### 解决步骤

#### 方法一：将用户添加到 dialout 组（推荐）

这是**永久性解决方案**，只需设置一次。

1. **添加当前用户到 dialout 组**：
   ```bash
   sudo usermod -a -G dialout $USER
   ```
   
   或者手动指定用户名：
   ```bash
   sudo usermod -a -G dialout your_username
   ```

2. **验证是否添加成功**：
   ```bash
   groups $USER
   ```
   
   输出中应该包含 `dialout`：
   ```
   your_username : your_username adm dialout cdrom sudo dip plugdev
   ```

3. **重要：注销并重新登录**
   
   权限更改需要重新登录才能生效。可以：
   - 注销当前会话后重新登录
   - 重启系统
   - 或者使用以下命令在当前终端临时生效：
     ```bash
     newgrp dialout
     ```

4. **验证权限**：
   ```bash
   ls -l /dev/ttyUSB0
   ```
   
   应该能正常访问串口设备。

#### 方法二：使用 sudo 运行（临时方案）

**注意**：此方法不推荐作为长期方案，仅用于测试。

```bash
sudo ./LockBoardTester
```

或者 AppImage：
```bash
sudo ./LockBoardTester-*.AppImage
```

**缺点**：
- 每次都需要输入密码
- 以 root 权限运行程序存在安全风险
- 配置文件可能保存到 root 用户目录

#### 方法三：临时修改串口权限（重启失效）

仅用于临时测试：
```bash
sudo chmod 666 /dev/ttyUSB0
```

此方法在设备重新插拔或系统重启后失效。

#### 方法四：创建 udev 规则（高级用户）

创建永久性的设备权限规则：

1. **创建 udev 规则文件**：
   ```bash
   sudo nano /etc/udev/rules.d/99-serial.rules
   ```

2. **添加以下内容**：
   ```
   # USB转RS485设备权限规则
   SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE="0666"
   SUBSYSTEM=="tty", KERNEL=="ttyUSB*", MODE="0666"
   SUBSYSTEM=="tty", KERNEL=="ttyACM*", MODE="0666"
   ```

3. **重新加载 udev 规则**：
   ```bash
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```

4. **重新插拔 USB 设备**

**查找设备 ID**：
```bash
lsusb
```
输出示例：
```
Bus 001 Device 005: ID 1a86:7523 QinHeng Electronics HL-340 USB-Serial adapter
```
其中 `1a86` 是 idVendor，`7523` 是 idProduct。

---

## 设备未识别问题

### 现象描述

- 插入 USB 转 RS485 设备后，刷新串口列表为空
- 系统中找不到 `/dev/ttyUSB*` 或 `/dev/ttyACM*` 设备
- `dmesg` 显示设备连接但未创建串口节点
- 设备管理器中无对应串口设备

### 原因分析

可能的原因包括：
1. 缺少驱动程序（特别是 CH340/CH341 芯片）
2. 驱动模块未加载
3. USB 设备故障或接触不良
4. 系统内核版本过旧
5. USB 控制器问题

### 解决步骤

#### 步骤 1：检查设备是否被识别

```bash
# 查看 USB 设备列表
lsusb

# 查看内核消息（插拔设备后立即执行）
dmesg | tail -30

# 查看串口设备列表
ls -l /dev/tty{USB,ACM}*
```

**正常输出示例**：
```bash
$ lsusb
Bus 001 Device 005: ID 1a86:7523 QinHeng Electronics HL-340 USB-Serial adapter

$ dmesg | tail -5
[ 1234.567890] usb 1-1: new full-speed USB device number 5 using xhci_hcd
[ 1234.678901] usb 1-1: New USB device found, idVendor=1a86, idProduct=7523
[ 1234.789012] ch341 1-1:1.0: ch341-uart converter detected
[ 1234.890123] usb 1-1: ch341-uart converter now attached to ttyUSB0
```

#### 步骤 2：检查并加载驱动

**检查驱动是否已加载**：
```bash
lsmod | grep -E 'ch341|pl2303|ftdi_sio|cp210x'
```

**常见芯片及对应驱动**：
| 芯片型号 | 驱动模块 | 制造商 |
|---------|---------|--------|
| CH340/CH341 | ch341 | 沁恒（WCH） |
| PL2303 | pl2303 | Prolific |
| FTDI FT232 | ftdi_sio | FTDI |
| CP2102/CP2104 | cp210x | Silicon Labs |

**手动加载驱动**：
```bash
# CH340/CH341
sudo modprobe ch341

# PL2303
sudo modprobe pl2303

# FTDI
sudo modprobe ftdi_sio

# CP210x
sudo modprobe cp210x
```

**设置开机自动加载**：
```bash
echo "ch341" | sudo tee -a /etc/modules
```

#### 步骤 3：安装缺失的驱动

**Ubuntu/Debian**：
```bash
# 更新包列表
sudo apt update

# 安装内核头文件和编译工具
sudo apt install linux-headers-$(uname -r) build-essential

# 重新插拔设备
```

**Fedora/RHEL/CentOS**：
```bash
# 安装内核开发包
sudo dnf install kernel-devel kernel-headers

# 或者 CentOS/RHEL 7
sudo yum install kernel-devel kernel-headers
```

**Arch Linux**：
```bash
sudo pacman -S linux-headers
```

#### 步骤 4：处理 CH340/CH341 特殊情况

某些系统需要手动编译 CH340 驱动：

```bash
# 下载驱动源码
wget https://github.com/juliagoda/CH341SER/archive/master.zip
unzip master.zip
cd CH341SER-master

# 编译安装
make
sudo make install

# 加载驱动
sudo modprobe ch341
```

#### 步骤 5：检查 USB 端口

```bash
# 查看 USB 控制器状态
lsusb -t

# 尝试其他 USB 端口
# 优先使用 USB 2.0 端口而非 USB 3.0
```

#### 步骤 6：检查设备硬件

- 更换 USB 数据线
- 尝试其他电脑验证设备是否正常
- 检查转换器的 LED 指示灯

### 验证解决方案

```bash
# 查看串口设备
ls -l /dev/ttyUSB*

# 测试串口读写（按 Ctrl+C 退出）
cat /dev/ttyUSB0

# 使用 minicom 测试
sudo apt install minicom
minicom -D /dev/ttyUSB0 -b 9600
```

---

## Qt 版本不匹配问题

### 现象描述

- 程序启动时报错：`version 'Qt_5.15' not found`
- 提示：`error while loading shared libraries: libQt5Widgets.so.5`
- AppImage 无法运行，提示缺少 Qt 库
- 程序界面异常或崩溃

### 原因分析

1. 系统安装的 Qt 版本与程序编译时使用的版本不一致
2. Qt 库路径未正确配置
3. AppImage 未能正确打包 Qt 依赖
4. 系统 Qt 版本过旧（低于 5.15）

### 解决步骤

#### 方法一：使用 AppImage（推荐）

AppImage 包含所有依赖，无需安装 Qt：

```bash
# 下载或构建 AppImage
./scripts/build-appimage.sh

# 添加执行权限
chmod +x LockBoardTester-*.AppImage

# 直接运行
./LockBoardTester-*.AppImage
```

**如果 AppImage 仍然报错**：
```bash
# 提取 AppImage 内容
./LockBoardTester-*.AppImage --appimage-extract

# 运行提取后的程序
./squashfs-root/AppRun
```

#### 方法二：检查 Qt 版本

```bash
# 检查系统 Qt 版本
qmake --version

# 查看已安装的 Qt 库
dpkg -l | grep libqt5  # Debian/Ubuntu
rpm -qa | grep qt5     # Fedora/RHEL
pacman -Q | grep qt5   # Arch Linux

# 查看库文件位置
ldconfig -p | grep Qt5
```

#### 方法三：安装/升级 Qt 5.15

**Ubuntu 20.04+**：
```bash
sudo apt update
sudo apt install qt5-default libqt5serialport5 libqt5serialport5-dev
```

**Ubuntu 22.04+**（qt5-default 已移除）：
```bash
sudo apt install qtbase5-dev qtbase5-dev-tools libqt5serialport5 libqt5serialport5-dev
```

**Fedora**：
```bash
sudo dnf install qt5-qtbase qt5-qtbase-devel qt5-qtserialport qt5-qtserialport-devel
```

**Arch Linux**：
```bash
sudo pacman -S qt5-base qt5-serialport
```

**从源码编译 Qt（高级用户）**：
```bash
# 下载 Qt 5.15.2 源码
wget https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz
tar xf qt-everywhere-src-5.15.2.tar.xz
cd qt-everywhere-src-5.15.2

# 配置（最小化安装）
./configure -opensource -confirm-license -nomake examples -nomake tests \
    -prefix /opt/qt5.15.2

# 编译（需要 2-4 小时）
make -j$(nproc)
sudo make install

# 设置环境变量
export PATH=/opt/qt5.15.2/bin:$PATH
export LD_LIBRARY_PATH=/opt/qt5.15.2/lib:$LD_LIBRARY_PATH
```

#### 方法四：配置库路径

如果 Qt 已安装但找不到：

```bash
# 查找 Qt 库位置
find /usr -name "libQt5Widgets.so*" 2>/dev/null

# 添加到库搜索路径
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# 永久设置
echo 'export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

#### 方法五：重新编译程序

使用系统安装的 Qt 版本重新编译：

```bash
# 清理旧的编译文件
make clean
rm -f Makefile*

# 重新生成 Makefile
qmake LockBoardTester.pro

# 编译
make -j$(nproc)

# 运行
./release/LockBoardTester
```

### 验证解决方案

```bash
# 检查程序依赖
ldd ./LockBoardTester | grep Qt5

# 正常输出应显示所有 Qt 库都已找到
libQt5Widgets.so.5 => /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5 (0x00007f...)
libQt5Gui.so.5 => /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5 (0x00007f...)
libQt5SerialPort.so.5 => /usr/lib/x86_64-linux-gnu/libQt5SerialPort.so.5 (0x00007f...)
```

---

## 中文字体显示问题

### 现象描述

- 界面中文显示为方框 `□□□`
- 中文字符显示为乱码
- 文字显示不完整或重叠
- 按钮和标签的中文文本消失

### 原因分析

1. 系统未安装中文字体
2. Qt 无法找到合适的中文字体
3. 字体配置不正确
4. locale 设置问题

### 解决步骤

#### 步骤 1：安装中文字体

**Ubuntu/Debian**：
```bash
# 安装常用中文字体
sudo apt update
sudo apt install fonts-wqy-microhei fonts-wqy-zenhei

# 或者安装 Noto 中文字体
sudo apt install fonts-noto-cjk fonts-noto-cjk-extra

# 或者安装文泉驿全部字体
sudo apt install xfonts-wqy ttf-wqy-microhei ttf-wqy-zenhei
```

**Fedora/RHEL/CentOS**：
```bash
sudo dnf install wqy-microhei-fonts wqy-zenhei-fonts

# 或者
sudo dnf install google-noto-sans-cjk-fonts google-noto-serif-cjk-fonts
```

**Arch Linux**：
```bash
sudo pacman -S wqy-microhei wqy-zenhei

# 或者
sudo pacman -S noto-fonts-cjk
```

**openSUSE**：
```bash
sudo zypper install wqy-microhei-fonts wqy-zenhei-fonts
```

#### 步骤 2：刷新字体缓存

```bash
# 更新字体缓存
sudo fc-cache -fv

# 验证中文字体
fc-list :lang=zh
```

**正常输出示例**：
```
/usr/share/fonts/truetype/wqy/wqy-microhei.ttc: WenQuanYi Micro Hei:style=Regular
/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc: WenQuanYi Zen Hei:style=Regular
```

#### 步骤 3：设置 locale

```bash
# 检查当前 locale
locale

# 安装中文 locale
sudo locale-gen zh_CN.UTF-8

# 临时设置环境变量
export LANG=zh_CN.UTF-8
export LC_ALL=zh_CN.UTF-8

# 永久设置（添加到 ~/.bashrc）
echo 'export LANG=zh_CN.UTF-8' >> ~/.bashrc
echo 'export LC_ALL=zh_CN.UTF-8' >> ~/.bashrc
source ~/.bashrc
```

**Debian/Ubuntu 系统设置**：
```bash
sudo dpkg-reconfigure locales
# 在界面中选择 zh_CN.UTF-8
```

#### 步骤 4：配置 Qt 字体

创建 Qt 配置文件：

```bash
# 创建配置目录
mkdir -p ~/.config/fontconfig

# 创建字体配置文件
cat > ~/.config/fontconfig/fonts.conf << 'EOF'
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
  <match target="pattern">
    <test qual="any" name="family">
      <string>sans-serif</string>
    </test>
    <edit name="family" mode="prepend" binding="strong">
      <string>WenQuanYi Micro Hei</string>
      <string>Noto Sans CJK SC</string>
    </edit>
  </match>
</fontconfig>
EOF

# 刷新配置
fc-cache -fv
```

#### 步骤 5：验证修复

```bash
# 重新启动程序
./LockBoardTester

# 或使用指定字体启动
QT_FONT_DPI=96 ./LockBoardTester
```

### 临时测试方案

```bash
# 强制使用特定字体
export QT_QPA_PLATFORMTHEME=gtk3
export QT_FONT_DPI=96

# 运行程序
./LockBoardTester
```

---

## 各发行版特定问题

### Ubuntu 系统

#### 问题：Wayland 会话下界面异常

**现象**：程序窗口无法正常显示或频繁闪烁

**解决方案**：
```bash
# 切换到 X11 会话
# 登录界面点击用户名后，右下角选择"Ubuntu on Xorg"

# 或强制使用 XCB 平台
export QT_QPA_PLATFORM=xcb
./LockBoardTester
```

#### 问题：Ubuntu 22.04 缺少 qt5-default

**解决方案**：
```bash
sudo apt install qtbase5-dev qtbase5-dev-tools \
    libqt5serialport5 libqt5serialport5-dev \
    qtchooser qt5-qmake
```

### Fedora 系统

#### 问题：SELinux 阻止串口访问

**现象**：即使加入 dialout 组仍无法访问串口

**解决方案**：
```bash
# 临时禁用 SELinux（不推荐）
sudo setenforce 0

# 或添加 SELinux 策略
sudo ausearch -c 'LockBoardTeste' --raw | audit2allow -M my-lockboard
sudo semodule -i my-lockboard.pp

# 或永久关闭 SELinux（需重启）
sudo sed -i 's/SELINUX=enforcing/SELINUX=permissive/' /etc/selinux/config
```

#### 问题：防火墙规则

如果使用网络串口服务器：
```bash
sudo firewall-cmd --permanent --add-port=9600/tcp
sudo firewall-cmd --reload
```

### Arch Linux 系统

#### 问题：滚动更新后程序无法启动

**解决方案**：
```bash
# 更新所有依赖
sudo pacman -Syu

# 重新编译程序
cd /path/to/lockBoard
make clean
qmake && make
```

#### 问题：缺少 uucp 组

Arch Linux 使用 `uucp` 而非 `dialout`：
```bash
sudo usermod -a -G uucp $USER
```

### Raspberry Pi / ARM 架构

#### 问题：性能问题

**解决方案**：
```bash
# 使用硬件加速
export QT_QPA_EGLFS_INTEGRATION=eglfs_kms

# 降低界面刷新率
export QT_QPA_UPDATE_IDLE_TIME=100
```

#### 问题：串口设备名称不同

树莓派串口通常是 `/dev/ttyAMA0` 或 `/dev/serial0`：
```bash
# 启用串口
sudo raspi-config
# 选择 Interface Options -> Serial Port
# 禁用登录 shell，启用串口硬件

# 检查串口
ls -l /dev/serial*
```

### openSUSE 系统

#### 问题：缺少 dialout 组

openSUSE 使用不同的组名：
```bash
# 查看串口所属组
ls -l /dev/ttyUSB0

# 添加到对应的组（通常是 dialout 或 uucp）
sudo usermod -a -G dialout $USER
```

---

## 通用调试技巧

### 启用 Qt 调试输出

```bash
# 设置调试级别
export QT_LOGGING_RULES="*.debug=true;qt.qpa.*=true"

# 运行程序并查看详细日志
./LockBoardTester 2>&1 | tee debug.log
```

### 检查依赖库

```bash
# 查看程序依赖
ldd ./LockBoardTester

# 检查缺失的库
ldd ./LockBoardTester | grep "not found"
```

### 使用 strace 追踪系统调用

```bash
# 追踪串口操作
strace -e trace=open,openat,read,write,ioctl ./LockBoardTester 2>&1 | grep tty
```

### 查看系统日志

```bash
# 实时查看内核消息
sudo dmesg -w

# 查看系统日志
sudo journalctl -xe -f
```

---

## 获取帮助

如果以上方案都无法解决问题，请收集以下信息并提交 Issue：

1. **系统信息**：
   ```bash
   uname -a
   lsb_release -a
   ```

2. **Qt 版本**：
   ```bash
   qmake --version
   ```

3. **设备信息**：
   ```bash
   lsusb
   ls -l /dev/tty{USB,ACM}*
   dmesg | tail -50
   ```

4. **错误日志**：
   ```bash
   ./LockBoardTester 2>&1 | tee error.log
   ```

5. **依赖检查**：
   ```bash
   ldd ./LockBoardTester
   ```

将以上输出保存并提交到项目的 Issue 页面。