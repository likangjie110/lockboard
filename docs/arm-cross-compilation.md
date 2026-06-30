# ARM 交叉编译指南

## 概述

本文档介绍如何为 ARM 架构设备（如树莓派、工控机、嵌入式 Linux 设备）交叉编译 LockBoardTester。

## 支持的架构

- **ARMv7 (32位)**: `armv7l`, `armhf` - 适用于树莓派 2/3/4（32位系统）
- **AArch64 (64位)**: `aarch64`, `arm64` - 适用于树莓派 3/4（64位系统）、工控机

## 快速开始

### 基础交叉编译（推荐）

```bash
# ARM 32位
./scripts/build-arm-cross.sh armv7l 1.0.0

# ARM 64位
./scripts/build-arm-cross.sh aarch64 1.0.0

# 静态链接版本（无需目标设备安装 Qt）
./scripts/build-arm-cross.sh armv7l 1.0.0 --static
```

### 输出

- 构建目录: `build-arm-{arch}/`
- 发布包: `dist/LockBoardTester-{version}-linux-{arch}.tar.gz`

## 环境准备

### 1. 安装交叉编译工具链

#### Ubuntu/Debian

```bash
# ARM 32位工具链
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# ARM 64位工具链
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# 验证安装
arm-linux-gnueabihf-gcc --version
aarch64-linux-gnu-gcc --version
```

#### Fedora/RHEL

```bash
# ARM 32位工具链
sudo dnf install gcc-arm-linux-gnu gcc-c++-arm-linux-gnu

# ARM 64位工具链
sudo dnf install gcc-aarch64-linux-gnu gcc-c++-aarch64-linux-gnu
```

#### Arch Linux

```bash
# ARM 工具链（AUR）
yay -S arm-linux-gnueabihf-gcc
yay -S aarch64-linux-gnu-gcc
```

### 2. Qt 交叉编译版本（可选但推荐）

#### 选项 A: 使用主机 Qt + 交叉编译器（简单但可能有依赖问题）

脚本会自动使用主机的 Qt，但生成的二进制需要目标设备安装相同版本的 Qt。

#### 选项 B: 编译 Qt 交叉版本（复杂但最佳实践）

**前提条件**：
- 需要目标设备的 sysroot（包含系统库和头文件）
- 预计编译时间：2-4 小时

**步骤**：

```bash
# 1. 下载 Qt 源码
wget https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz
tar xf qt-everywhere-src-5.15.2.tar.xz
cd qt-everywhere-src-5.15.2

# 2. 配置交叉编译（ARM 32位示例）
./configure \
    -prefix /opt/qt5-armv7 \
    -device linux-rasp-pi3-g++ \
    -device-option CROSS_COMPILE=arm-linux-gnueabihf- \
    -sysroot /path/to/target-sysroot \
    -opensource -confirm-license \
    -release -optimized-qmake \
    -no-opengl -no-xcb \
    -nomake examples -nomake tests \
    -skip qtwebengine -skip qt3d

# 3. 编译并安装
make -j$(nproc)
sudo make install

# 4. 设置环境变量
export QT_CROSS_PATH=/opt/qt5-armv7
```

**获取目标 sysroot 的方法**：

```bash
# 方法 1: 从目标设备复制
rsync -avz --rsync-path="sudo rsync" \
  user@target:/lib \
  user@target:/usr/include \
  user@target:/usr/lib \
  ./target-sysroot/

# 方法 2: 使用 crosstool-NG 生成
# 方法 3: 下载预构建的 sysroot（树莓派官方）
```

## 使用脚本

### 命令格式

```bash
./scripts/build-arm-cross.sh [ARCH] [VERSION] [OPTIONS]
```

### 参数说明

**架构选项**：
- `armv7l`, `armv7`, `arm32` - ARM 32位
- `aarch64`, `arm64` - ARM 64位

**其他参数**：
- `VERSION` - 版本号（默认: 1.0.0）
- `--static` - 静态链接（需要静态版 Qt）
- `--help` - 显示帮助信息

### 示例

```bash
# 基础编译（动态链接）
./scripts/build-arm-cross.sh armv7l 1.0.0

# 64位版本
./scripts/build-arm-cross.sh aarch64 1.0.1

# 静态链接（推荐用于发布）
./scripts/build-arm-cross.sh armv7l 1.0.0 --static

# 使用自定义 Qt 路径
QT_CROSS_PATH=/opt/qt5-armv7 ./scripts/build-arm-cross.sh armv7l 1.0.0

# 使用自定义工具链
CROSS_COMPILE_PREFIX=arm-none-linux-gnueabihf- \
  ./scripts/build-arm-cross.sh armv7l 1.0.0
```

## 环境变量

脚本支持以下环境变量自定义配置：

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `QT_CROSS_PATH` | Qt 交叉编译版本路径 | `/opt/qt5-{arch}` |
| `CROSS_COMPILE_PREFIX` | 工具链前缀 | `arm-linux-gnueabihf-` / `aarch64-linux-gnu-` |
| `SYSROOT` | 目标系统根目录 | 无 |

## 部署到目标设备

### 1. 传输文件

```bash
# 使用 scp
scp dist/LockBoardTester-1.0.0-linux-armv7l.tar.gz user@target:/tmp/

# 或使用 rsync
rsync -avz dist/LockBoardTester-1.0.0-linux-armv7l.tar.gz user@target:/tmp/
```

### 2. 在目标设备上安装

```bash
# SSH 登录到目标设备
ssh user@target

# 解压
cd /tmp
tar xzf LockBoardTester-1.0.0-linux-armv7l.tar.gz
cd lockboard-1.0.0-linux-armv7l

# 添加串口权限（首次运行）
sudo usermod -a -G dialout $USER
# 注销并重新登录后生效

# 安装运行时依赖（动态链接版本）
sudo apt install libqt5core5a libqt5gui5 libqt5widgets5 libqt5serialport5

# 运行
./run.sh
```

### 3. 开机自启动（可选）

```bash
# 创建 systemd 服务
sudo nano /etc/systemd/system/lockboard.service
```

内容：
```ini
[Unit]
Description=LockBoard Tester
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/opt/lockboard
ExecStart=/opt/lockboard/run.sh
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

启用服务：
```bash
sudo systemctl enable lockboard
sudo systemctl start lockboard
```

## 常见目标设备配置

### 树莓派

| 型号 | 架构 | 系统 | 推荐编译选项 |
|------|------|------|--------------|
| 树莓派 2 | armv7l | 32位 | `armv7l` |
| 树莓派 3 | armv7l / aarch64 | 32/64位 | `armv7l` 或 `aarch64` |
| 树莓派 4 | aarch64 | 64位 | `aarch64` |
| 树莓派 Zero | armv6l | 32位 | 不支持（需特殊工具链）|

### 工控机/嵌入式设备

| 品牌 | 典型架构 | 示例型号 |
|------|----------|----------|
| 研华 | ARM Cortex-A | ARK-1123, MIO-5373 |
| 威强电 | ARM / x86 | WAFER-APLP4 |
| 研祥 | ARM | EPC-1816 |

**注意**：具体架构请查看设备手册或运行 `uname -m` 命令。

## 故障排查

### 问题 1: 找不到交叉编译器

**错误信息**：
```
未找到交叉编译器: arm-linux-gnueabihf-gcc
```

**解决方案**：
```bash
# 检查是否安装
which arm-linux-gnueabihf-gcc

# 如果未安装
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

### 问题 2: qmake 配置失败

**错误信息**：
```
qmake 配置失败
```

**解决方案**：
1. 确认 Qt 开发包已安装：`sudo apt install qt5-qmake qtbase5-dev`
2. 检查 Qt 路径：`qmake -query QT_INSTALL_PREFIX`
3. 使用环境变量指定路径：`QT_CROSS_PATH=/path/to/qt ./scripts/build-arm-cross.sh`

### 问题 3: 目标设备运行时缺少库

**错误信息**（在目标设备上）：
```
error while loading shared libraries: libQt5Core.so.5: cannot open shared object file
```

**解决方案**：

```bash
# 方案 1: 安装 Qt 运行时
sudo apt install libqt5core5a libqt5gui5 libqt5widgets5 libqt5serialport5

# 方案 2: 使用静态编译
./scripts/build-arm-cross.sh armv7l 1.0.0 --static

# 方案 3: 手动复制库文件
mkdir -p dist/lockboard-1.0.0-linux-armv7l/lib
cp /path/to/qt/lib/libQt5*.so.5 dist/lockboard-1.0.0-linux-armv7l/lib/
```

### 问题 4: 架构不匹配

**错误信息**（在目标设备上）：
```
cannot execute binary file: Exec format error
```

**解决方案**：
1. 检查目标设备架构：`uname -m`
2. 重新编译匹配的架构：
   - 如果设备显示 `armv7l` → 使用 `armv7l`
   - 如果设备显示 `aarch64` → 使用 `aarch64`

### 问题 5: GLIBC 版本不兼容

**错误信息**：
```
version `GLIBC_2.29' not found
```

**解决方案**：
1. 在编译主机上使用较旧的发行版（如 Ubuntu 18.04）
2. 使用目标设备的 sysroot 进行编译
3. 考虑使用静态链接

## 性能优化建议

### 编译时优化

在 `.pro` 文件中已包含优化选项：
- `-O3`: 高级优化
- `-s`: 去除调试符号

### 进一步优化（可选）

编辑 `LockBoardTester.pro`，针对 ARM 添加：

```qmake
# ARM 特定优化
linux-arm* {
    # ARMv7 NEON 优化
    contains(QMAKE_HOST.arch, armv7.*) {
        QMAKE_CXXFLAGS += -mfpu=neon -mfloat-abi=hard
    }
    
    # 链接时优化
    QMAKE_LFLAGS += -Wl,-O1 -Wl,--as-needed
}
```

## 参考资源

### 官方文档
- [Qt for Embedded Linux](https://doc.qt.io/qt-5/embedded-linux.html)
- [Qt Configure Options](https://doc.qt.io/qt-5/configure-options.html)

### 工具链文档
- [ARM GNU Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
- [Linaro Toolchain](https://www.linaro.org/downloads/)

### 树莓派资源
- [Raspberry Pi Cross-compilation](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#cross-compiling-the-kernel)
- [Qt on Raspberry Pi](https://wiki.qt.io/RaspberryPi)

## 支持和反馈

如遇到问题，请提供以下信息：
1. 编译主机系统和版本
2. 目标设备架构（`uname -m` 输出）
3. Qt 版本（`qmake -query QT_VERSION`）
4. 完整的错误日志