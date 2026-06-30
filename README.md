# 485锁控板通讯测试工具

[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-blue)](https://github.com)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-5.15.2-brightgreen)](https://www.qt.io/)
[![Release](https://img.shields.io/github/v/release/YOUR_USERNAME/lockBoard)](https://github.com/YOUR_USERNAME/lockBoard/releases)
[![Build](https://img.shields.io/github/actions/workflow/status/YOUR_USERNAME/lockBoard/release.yml)](https://github.com/YOUR_USERNAME/lockBoard/actions)

[English](README_EN.md) | 简体中文

## 项目概述

基于Qt 5.15.2开发的RS485锁控板通讯测试工具，支持12路锁控制和完整的协议命令验证。

### 项目截图

<div align="center">
  <img src="images/main-window.png" alt="主界面" width="45%">
  <img src="images/debug-console.png" alt="调试控制台" width="45%">
</div>
<div align="center">
  <img src="images/lock-status.png" alt="锁状态显示" width="45%">
</div>

> 💡 提示：截图占位，实际运行界面请参考程序

## 平台支持

| 平台 | 状态 | 说明 |
|------|------|------|
| ✅ Windows | 完全支持 | Windows 10/11，预编译可执行文件 |
| ✅ Linux | 完全支持 | Ubuntu/Debian/Fedora/Arch，需从源码编译 |
| ⚠️ macOS | 理论支持 | 未测试，需自行编译 |

## 📥 下载

### 预编译版本

访问 [Releases 页面](https://github.com/YOUR_USERNAME/lockBoard/releases) 下载最新版本：

| 平台 | 文件 | 推荐用户 |
|------|------|----------|
| **Windows 64位** | `LockBoardTester-Windows-x64.exe` | ✅ 推荐：现代 Windows 10/11 系统 |
| **Windows 32位** | `LockBoardTester-Windows-x86.exe` | 老旧 32 位系统 |
| **Linux x64** | `LockBoardTester-Linux-x86_64.AppImage` | 标准 Linux 发行版（无需安装） |
| **Linux ARM64** | `LockBoardTester-Linux-arm64` | 树莓派 4、Jetson Nano 等 |
| **Linux ARMv7** | `LockBoardTester-Linux-armv7` | 树莓派 3 等 |

**使用方法**：
- **Windows**: 下载 `.exe` 文件，双击运行
- **Linux AppImage**: `chmod +x *.AppImage && ./*.AppImage`
- **Linux ARM**: `chmod +x LockBoardTester-* && ./LockBoardTester-*`

### Linux 快速开始

```bash
# 1. 克隆或下载项目
cd lockBoard

# 2. 运行自动化安装脚本（推荐）
sudo bash scripts/setup-linux.sh

# 3. 编译项目
qmake LockBoardTester.pro
make -j$(nproc)

# 4. 运行程序
./LockBoardTester
```

**详细安装说明**：请查看 [Linux 安装指南](docs/linux-setup.md)

## 文件说明

- **可执行文件**: `release/LockBoardTester.exe`
- **源代码目录** (`src/`):
  - `main.cpp` - 程序入口
  - `mainwindow.h/cpp` - 主窗口界面
  - `protocol/protocol485.h/cpp` - 485协议封装（9种命令支持）
  - `serial/serialmanager.h/cpp` - 串口管理（9600波特率，8N1配置）
  - `ui/mainwindow.ui` - Qt界面设计文件
  - `ui/lockstatuswidget.h/cpp` - 12路锁状态可视化组件
  - `ui/debugconsole.h/cpp` - 调试控制台（十六进制日志）
- **项目配置**: `LockBoardTester.pro`
- **协议文档**: `protocol.txt` / `485锁控板通讯协议 -12(1)(1).pdf`

## 功能特性

### 支持的协议命令

| 命令码 | 功能 | 说明 |
|--------|------|------|
| 0x01 | 获取版本号 | 查询单片机固件版本 |
| 0x10 | 开单个锁 | 打开指定通道的锁 |
| 0x11 | 读单个锁状态 | 读取指定锁的开关状态 |
| 0x12 | 读所有锁状态 | 一次性读取12路锁状态 |
| 0x14 | 开全部锁 | 同时打开所有12路锁 |
| 0x15 | 开多个锁 | 按位选择多个锁打开 |
| 0x16 | 持续输出控制 | 用于控制继电器/照明灯等持续通电设备 |
| 0x18 | 同时开多个锁 | 专用于同一门多锁场景 |
| 0x1A | 多通道输出控制 | 批量控制多个通道输出状态 |

### 界面布局

**左侧 - 连接配置区**
- 串口选择下拉框（自动扫描可用串口）
- 刷新串口按钮
- 连接/断开按钮
- 板卡地址选择（1-15）

**中间 - 功能测试区（5个标签页）**
1. **基础功能**: 获取版本、开全部锁、读所有状态
2. **单路控制**: 选择通道、开单锁、读状态
3. **多路控制**: 12个复选框批量选择
4. **持续输出**: 通道选择、开关控制
5. **多通道输出**: 批量输出控制

**右上 - 锁状态显示**
- 12个可视化指示器（3×4网格布局）
- 颜色标识：
  - 🟢 绿色 = 锁打开
  - 🔴 红色 = 锁关闭
  - ⚪ 灰色 = 未知/无响应
- 实时状态更新

**右下 - 调试控制台**
- 发送数据（蓝色标记）
- 接收数据（绿色标记）
- 错误信息（红色标记）
- 十六进制格式显示
- 时间戳记录
- 清空/保存日志功能

## 使用方法

### 1. 硬件连接

```
PC (USB) <---> USB转RS485转换器 <---> 锁控板
```

- 波特率: 9600
- 数据位: 8
- 停止位: 1
- 校验位: 无

### 2. 启动程序

运行 `release/LockBoardTester.exe`

### 3. 建立连接

1. 点击"刷新串口"扫描可用串口
2. 选择对应的COM口
3. 设置板卡地址（对应硬件拨码开关设置）
4. 点击"连接"按钮

### 4. 测试功能

#### 验证通讯
- 点击"获取版本"按钮
- 查看调试控制台是否收到回复
- 确认通讯正常

#### 开锁测试
- 切换到"单路控制"标签
- 选择锁通道（1-12）
- 点击"开锁"
- 观察状态指示器变化

#### 批量操作
- 切换到"多路控制"标签
- 勾选需要操作的锁
- 点击"开多个锁"
- 查看调试日志和状态更新

## 协议格式

### 发送帧格式
```
[0xDC] [0x02] [地址] [命令] [长度] [数据段] [校验]
```

### 接收帧格式
```
[0xDC] [0xFE] [地址] [命令] [长度] [数据段] [校验]
```

### 校验计算
```
校验值 = (地址 + 命令 + 长度 + 数据段各字节) % 256
```

### 示例：开1号锁
```
发送: DC 02 01 10 01 01 13
      │   │  │  │  │  │  └─ 校验值 (01+10+01+01=13)
      │   │  │  │  │  └──── 锁ID=1
      │   │  │  │  └─────── 数据长度=1
      │   │  │  └────────── 命令=0x10
      │   │  └───────────── 地址=1
      │   └──────────────── 包头0x02
      └──────────────────── 包头0xDC

回复: DC FE 01 10 02 01 01 15
      │   │  │  │  │  │  │  └─ 校验值
      │   │  │  │  │  │  └──── 状态=1(成功)
      │   │  │  │  │  └─────── 锁ID=1
      │   │  │  │  └────────── 数据长度=2
      │   │  │  └───────────── 命令=0x10
      │   │  └──────────────── 地址=1
      │   └─────────────────── 包头0xFE
      └─────────────────────── 包头0xDC
```

## 常见问题

### Q: 无法找到串口？
A: 
- 检查USB转RS485设备是否正确安装驱动
- 在设备管理器中确认COM口号
- 点击"刷新串口"按钮

### Q: 发送命令无响应？
A:
- 确认板卡地址设置正确（对应硬件拨码开关）
- 检查RS485接线（A接A，B接B）
- 查看调试控制台是否有错误信息
- 尝试断开重连

### Q: 状态指示器不更新？
A:
- 手动点击"读所有状态"刷新
- 检查通讯日志确认收到回复
- 验证数据帧格式是否正确

### Q: 如何测试多板卡级联？
A:
- 设置不同板卡的硬件地址（1-15）
- 在软件中切换地址进行通讯
- 每次只能与一个地址的板卡通讯

## 开发信息

- **开发环境**: Qt 5.15.2 + MinGW 8.1.0 64-bit
- **编译方式**: qmake + mingw32-make
- **依赖模块**: QtWidgets, QtGui, QtSerialPort, QtCore

## 重新编译

如需修改源码后重新编译：

```powershell
# 设置Qt环境
$env:PATH = "D:\QT\5.15.2\mingw81_64\bin;D:\QT\Tools\mingw810_64\bin;$env:PATH"

# 生成Makefile
qmake LockBoardTester.pro

# 编译
mingw32-make

# 可执行文件生成在 release/LockBoardTester.exe
```

## 打包和发布

### Windows 平台

#### 标准打包（带依赖文件夹）
```batch
# 自动编译并使用 windeployqt 收集依赖
scripts\build-windows-single.bat 1.0.0

# 输出目录: dist\
# 包含: LockBoardTester.exe 及所有 Qt DLL 和插件
```

#### 单文件打包（可选）
安装 [Enigma Virtual Box](https://enigmaprotector.com/en/downloads.html) 后：
```batch
scripts\build-windows-single.bat 1.0.0

# 输出: LockBoardTester-1.0.0-standalone.exe (单文件，约 8-15 MB)
```

#### 启用压缩优化（可选）
安装 [UPX](https://upx.github.io/) 后，脚本会自动压缩 DLL 文件，可减少约 40-60% 体积。

### Linux 平台

#### AppImage 打包（推荐）
```bash
# 基础打包
./scripts/build-appimage.sh 1.0.0

# 启用 UPX 压缩
./scripts/build-appimage.sh 1.0.0 --compress

# 输出: LockBoardTester-1.0.0-x86_64.AppImage
```

#### 查看帮助
```bash
./scripts/build-appimage.sh --help
```

### 跨平台统一打包

使用统一脚本自动检测系统并调用对应平台的打包工具：

```bash
# 基础打包
./scripts/build-all.sh --version 1.0.0

# 启用压缩并清理旧文件
./scripts/build-all.sh --version 1.0.0 --compress --clean

# 查看帮助
./scripts/build-all.sh --help
```

### ARM 平台交叉编译

针对嵌入式 Linux 设备（树莓派、工控机等）：

```bash
# ARM 32位（树莓派 2/3/4 32位系统）
./scripts/build-arm-cross.sh armv7l 1.0.0

# ARM 64位（树莓派 3/4 64位系统、工控机）
./scripts/build-arm-cross.sh aarch64 1.0.0

# 静态链接版本（推荐，无需目标设备安装 Qt）
./scripts/build-arm-cross.sh armv7l 1.0.0 --static

# 输出: dist/LockBoardTester-1.0.0-linux-armv7l.tar.gz
```

**部署到目标设备**：
```bash
# 传输文件
scp dist/LockBoardTester-*.tar.gz user@target:/tmp/

# SSH 登录并安装
ssh user@target
cd /tmp && tar xzf LockBoardTester-*.tar.gz
cd lockboard-*
./run.sh
```

详细的 ARM 交叉编译指南，请参考：
- [ARM 交叉编译文档](docs/arm-cross-compilation.md)

### 输出文件说明

| 平台 | 文件 | 大小 | 说明 |
|------|------|------|------|
| Windows | `dist/` 目录 | ~25-30 MB | 标准版本，包含所有依赖 |
| Windows | `LockBoardTester-*-standalone.exe` | ~8-15 MB | 单文件版本（需 Enigma VB） |
| Linux x86_64 | `LockBoardTester-*-x86_64.AppImage` | ~20-25 MB | 便携版本，无需安装 |
| Linux ARM | `LockBoardTester-*-linux-armv7l.tar.gz` | ~15-20 MB | ARM 32位交叉编译版本 |
| Linux ARM | `LockBoardTester-*-linux-aarch64.tar.gz` | ~15-20 MB | ARM 64位交叉编译版本 |

### 打包工具配置

详细的打包工具安装和配置指南，请参考：
- [打包工具配置文档](docs/packaging-setup.md)
- [ARM 交叉编译配置](docs/arm-cross-compilation.md)

## 技术支持

如有问题，请检查：
1. 协议文档: `protocol.txt`
2. 调试控制台的原始数据帧
3. 校验值计算是否正确

## 版本历史

- **v1.0.0** (2026-06-30)
  - 初始版本
  - 支持全部9种协议命令
  - 12路锁状态可视化
  - 调试控制台日志功能

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件