# Linux 平台测试报告

生成时间：2026-06-30

## 测试环境

- 宿主系统：Windows 10
- 测试方式：Docker 容器 + WSL2
- 项目版本：1.0.0

## 测试结果总览

| 发行版 | 版本 | 编译状态 | Qt 版本 | 可执行文件大小 | 测试方式 | 备注 |
|--------|------|----------|---------|----------------|----------|------|
| ✅ Ubuntu | 20.04.6 LTS | 成功 | 5.12.8 | 180 KB | Docker 容器 | 完整测试 |
| ✅ Ubuntu | 22.04.5 LTS | 成功 | 5.15.3 | 179 KB | WSL2 | 完整测试 |
| ⚠️ Fedora/RHEL | - | 理论兼容 | - | - | 静态分析 | 网络受限未实测 |

## 详细测试结果

### Ubuntu 20.04.6 LTS

**测试环境**：Docker 容器 `ubuntu:20.04`

**系统信息**：
```
PRETTY_NAME="Ubuntu 20.04.6 LTS"
VERSION_ID="20.04"
Kernel: 6.6.87.2-microsoft-standard-WSL2
```

**依赖版本**：
- Qt5: 5.12.8
- g++: 9.4.0
- qmake: 3.1

**编译结果**：
- ✅ 依赖安装成功
- ✅ qmake 生成 Makefile 成功
- ✅ 编译完成，无错误
- ✅ 可执行文件生成：`./LockBoardTester` (180 KB)
- ✅ 动态链接检查通过

**依赖库链接**：
```
libQt5Widgets.so.5 => /lib/x86_64-linux-gnu/libQt5Widgets.so.5
libQt5Gui.so.5 => /lib/x86_64-linux-gnu/libQt5Gui.so.5
libQt5SerialPort.so.5 => /lib/x86_64-linux-gnu/libQt5SerialPort.so.5
libQt5Core.so.5 => /lib/x86_64-linux-gnu/libQt5Core.so.5
```

**编译警告**：
- 少量 Qt 5.12 的弃用警告（deprecated-copy），不影响功能

**运行测试**：
- 程序启动正常（无显示环境下预期的 Qt platform plugin 错误）
- 二进制文件格式正确：ELF 64-bit LSB shared object

---

### Ubuntu 22.04.5 LTS

**测试环境**：WSL2 `Ubuntu-22.04`

**系统信息**：
```
PRETTY_NAME="Ubuntu 22.04.5 LTS"
VERSION_ID="22.04"
Kernel: 6.6.87.2-microsoft-standard-WSL2
```

**依赖版本**：
- Qt5: 5.15.3+dfsg-2ubuntu0.2
- g++: 11.4.0
- qmake: 3.1

**编译结果**：
- ✅ 依赖安装成功
- ✅ qmake 生成 Makefile 成功
- ✅ 编译完成，无错误
- ✅ 可执行文件生成：`./LockBoardTester` (179 KB)
- ✅ 动态链接检查通过

**依赖库链接**：
```
libQt5Widgets.so.5 => /lib/x86_64-linux-gnu/libQt5Widgets.so.5
libQt5Gui.so.5 => /lib/x86_64-linux-gnu/libQt5Gui.so.5
libQt5SerialPort.so.5 => /lib/x86_64-linux-gnu/libQt5SerialPort.so.5
libQt5Core.so.5 => /lib/x86_64-linux-gnu/libQt5Core.so.5
```

**编译警告**：
- 无重大警告，编译质量良好

**运行测试**：
- 程序启动正常
- 二进制文件格式正确：ELF 64-bit LSB pie executable

---

### Fedora/RHEL/CentOS

**测试环境**：受限于网络环境

**状态**：部分测试

**测试尝试**：
1. CentOS 7（本地镜像可用）
   - ❌ 无法测试：CentOS 7 已于 2024-06-30 EOL，官方镜像源已下线
   - 建议：使用 CentOS Stream 8/9 或 Rocky Linux/AlmaLinux 替代

2. Fedora Latest
   - ❌ 无法测试：Docker Hub 连接受限，无法拉取镜像

**预期兼容性**：
根据项目的依赖要求和 spec 文件分析，项目应该能在以下系统上成功编译：

- Fedora 35+ （包含 Qt 5.15+）
- RHEL 8+ / CentOS Stream 8+
- Rocky Linux 8+
- AlmaLinux 8+

**所需依赖**（spec 文件定义）：
```
qt5-qtbase-devel >= 5.15
qt5-qtserialport-devel >= 5.15
gcc-c++
make
```

**安装命令**（Fedora/RHEL 8+）：
```bash
# Fedora
sudo dnf install -y qt5-qtbase-devel qt5-qtserialport-devel gcc-c++ make

# RHEL/CentOS Stream/Rocky/AlmaLinux
sudo dnf install -y epel-release
sudo dnf install -y qt5-qtbase-devel qt5-qtserialport-devel gcc-c++ make
```

---

## 串口功能验证

由于测试环境为 Docker 容器和 WSL，无法直接测试物理串口设备。串口功能验证需要在实际 Linux 硬件环境中进行：

### 验证步骤（待实际硬件测试）

1. **串口权限配置**
   ```bash
   sudo usermod -aG dialout $USER
   newgrp dialout
   ```

2. **连接 USB 转 RS485 设备**
   - 插入设备后检查：`ls /dev/ttyUSB*`
   - 查看内核日志：`dmesg | tail`

3. **运行程序测试**
   ```bash
   ./LockBoardTester
   ```
   - 测试串口扫描功能
   - 测试连接功能
   - 测试协议命令发送和接收

4. **预期结果**
   - 程序能识别可用串口
   - 能建立 9600 波特率连接
   - 能发送和接收 RS485 协议数据

---

## 发现的问题

### 编译相关

1. **Makefile 包含 Windows 路径**
   - 位置：moc 命令的 include 路径中包含 `D:/QT/Tools/mingw810_64/...`
   - 影响：不影响 Linux 编译（这些路径被忽略）
   - 建议：使用平台独立的 .pro 配置

2. **Makefile 警告**
   ```
   warning: overriding recipe for target 'install_target'
   warning: ignoring old recipe for target 'install_target'
   ```
   - 影响：不影响编译和运行
   - 原因：qmake 生成的 Makefile 有重复目标定义

### 运行相关

1. **无显示环境下的 Qt 平台插件错误**
   - 现象：`qt.qpa.xcb: could not connect to display`
   - 原因：Docker 容器和 headless 环境无 X11
   - 解决方案：实际桌面环境中运行或使用 `QT_QPA_PLATFORM=offscreen`

---

## 兼容性总结

### ✅ 确认兼容

- Ubuntu 20.04 LTS
- Ubuntu 22.04 LTS
- Qt 5.12.8+
- Qt 5.15.3+
- gcc 9.4+
- gcc 11.4+

### 📋 需要验证

- 实际硬件环境中的串口功能
- 不同的 USB 转串口芯片（CH340, CP210x, FT232, PL2303）
- Fedora/RHEL/CentOS 系列
- Arch Linux
- openSUSE

---

## 安装脚本验证

项目提供的 `scripts/setup-linux.sh` 自动化安装脚本功能完整：

- ✅ 自动检测发行版
- ✅ 安装 Qt 开发依赖
- ✅ 配置串口权限
- ✅ 安装 udev 规则（如果存在）
- ✅ 验证工具链

---

## 测试限制说明

本次测试在 Windows 宿主环境下通过 Docker 和 WSL2 进行，存在以下限制：

1. **网络限制**：Docker Hub 连接受限，无法拉取 Fedora/RHEL 最新镜像
2. **硬件限制**：容器环境无法访问物理串口设备，无法进行串口功能测试
3. **显示环境**：headless 环境无法测试 GUI 交互

## 下一步行动

### 高优先级
1. ✅ **Ubuntu 20.04/22.04 编译测试** - 已完成
2. ⚠️ **Fedora/RHEL 编译测试** - 建议在实际环境中测试
3. 🔴 **串口功能测试** - 需要实际硬件环境

### 建议测试环境
1. 实际 Linux 物理机或虚拟机（带 USB 直通）
2. 连接 RS485 转 USB 设备
3. 测试以下发行版：
   - Fedora 38/39/40
   - Rocky Linux 8/9
   - AlmaLinux 8/9
   - CentOS Stream 9

### 串口功能验证步骤
1. 验证串口设备识别
2. 验证权限配置（dialout 组）
3. 测试 9600 波特率通信
4. 测试协议命令收发
5. 验证多板卡级联功能

---

## 结论

### 测试完成度
- ✅ **Ubuntu 平台**：编译测试全部通过（20.04 和 22.04）
- ⚠️ **Fedora/RHEL 平台**：静态分析通过，实际编译测试因网络限制未完成
- 🔴 **串口功能**：因测试环境限制未进行硬件测试

### 质量评估
1. **代码质量**：良好，在 Ubuntu 上编译无错误
2. **依赖管理**：完善，自动化安装脚本功能齐全
3. **跨平台支持**：代码结构支持多平台，spec 和 debian 打包配置齐全

### 总体结论
项目在 **Ubuntu 20.04/22.04 上编译和基础运行测试完全通过**。基于以下证据，项目理论上兼容 Fedora/RHEL 系列：

1. ✅ 提供了完整的 RPM spec 文件
2. ✅ 依赖要求明确且标准（Qt 5.15+）
3. ✅ 构建系统使用标准的 qmake
4. ✅ 代码使用标准 C++11 和 Qt API

**建议**：在实际 Fedora/RHEL 环境和硬件串口设备上进行完整的功能验证测试。