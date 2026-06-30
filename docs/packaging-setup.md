# LockBoardTester 打包工具配置指南

本文档介绍如何配置 Windows 和 Linux 平台的打包工具链。

---

## 目录

- [Windows 工具链](#windows-工具链)
  - [必需工具](#必需工具)
  - [可选工具](#可选工具)
- [Linux 工具链](#linux-工具链)
- [常见问题](#常见问题)

---

## Windows 工具链

### 必需工具

#### 1. Qt 开发环境

**推荐版本**: Qt 5.15.2 或更高

**下载地址**: 
- 官方: https://www.qt.io/download-open-source
- 国内镜像: https://mirrors.tuna.tsinghua.edu.cn/qt/

**安装选项**:
- Qt 5.15.2 (或其他版本)
  - MinGW 7.3.0 64-bit (推荐) 或 MSVC 2019 64-bit
  - Qt Charts (可选)
  - Qt Data Visualization (可选)
- Developer and Designer Tools
  - MinGW 7.3.0 64-bit (如果选择 MinGW)
  - Qt Creator

**配置环境变量**:
```batch
# 将以下路径添加到系统 PATH
C:\Qt\5.15.2\mingw81_64\bin
C:\Qt\Tools\mingw810_64\bin
```

**验证安装**:
```batch
qmake -version
# 输出: QMake version 3.1, Using Qt version 5.15.2 ...

mingw32-make --version
# 或 nmake (如果使用 MSVC)
```

#### 2. windeployqt

windeployqt 是 Qt 自带的部署工具，用于收集应用程序所需的所有 Qt 依赖。

**位置**: Qt 安装目录的 bin 文件夹中（如 `C:\Qt\5.15.2\mingw81_64\bin\windeployqt.exe`）

**验证**:
```batch
windeployqt --version
```

---

### 可选工具

#### 1. UPX (Ultimate Packer for eXecutables)

UPX 用于压缩 DLL 和可执行文件，可显著减小文件体积。

**推荐版本**: 4.0.0 或更高

**下载地址**: 
- GitHub: https://github.com/upx/upx/releases
- 官网: https://upx.github.io/

**安装步骤**:
1. 下载 `upx-<version>-win64.zip`
2. 解压到任意目录（如 `C:\Tools\upx\`）
3. 将 `upx.exe` 所在目录添加到系统 PATH

**配置环境变量**:
```batch
C:\Tools\upx
```

**验证安装**:
```batch
upx --version
# 输出: upx 4.0.0 ...
```

**压缩效果**:
- Qt5Core.dll: 约减少 50-60%
- Qt5Gui.dll: 约减少 40-50%
- Qt5Widgets.dll: 约减少 45-55%

**注意事项**:
- 压缩后的 DLL 可能触发某些杀毒软件误报
- 启动速度会略微降低（需要解压）
- 不影响功能和稳定性

#### 2. Enigma Virtual Box

Enigma Virtual Box 用于将多个文件打包成单个可执行文件。

**推荐版本**: 10.0 或更高

**下载地址**:
- 官网: https://enigmaprotector.com/en/downloads.html
- 直接下载: https://enigmaprotector.com/assets/files/enigmavb.exe

**安装步骤**:
1. 下载并运行安装程序
2. 选择安装目录（默认 `C:\Program Files (x86)\Enigma Virtual Box\`）
3. 完成安装

**命令行工具**:
构建脚本会自动使用命令行版本 `enigmavbconsole.exe`，无需手动配置。

**验证安装**:
```batch
"C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe"
# 应该显示帮助信息
```

**打包效果**:
- 所有 DLL 和资源文件打包到单个 exe
- 无需安装，双击即可运行
- 文件大小约 15-25 MB（未压缩）或 8-15 MB（UPX 压缩后）

**注意事项**:
- 仅支持 Windows 平台
- 首次运行会解压文件到临时目录（略微延迟）
- 某些杀毒软件可能需要添加白名单

---

## Linux 工具链

Linux 打包工具链配置请参考 [linux-setup.md](linux-setup.md) 文档。

**快速摘要**:

### 必需工具
- **Qt 5.15+**: `sudo apt install qt5-default qtbase5-dev libqt5serialport5-dev`
- **linuxdeployqt**: 脚本会自动下载

### 可选工具
- **UPX**: `sudo apt install upx-ucl` (Debian/Ubuntu)
- **ImageMagick**: `sudo apt install imagemagick` (用于生成图标)

---

## 常见问题

### Windows 平台

#### Q1: 提示 "未找到 qmake"
**解决方案**:
1. 确认 Qt 已正确安装
2. 检查环境变量 PATH 是否包含 Qt bin 目录
3. 重启命令提示符或 PowerShell

#### Q2: 编译时提示找不到 Qt 头文件
**解决方案**:
- MinGW 用户: 确保安装了 MinGW 版本的 Qt
- MSVC 用户: 
  1. 使用 "Developer Command Prompt for VS"
  2. 或配置 MSVC 环境变量

#### Q3: Enigma Virtual Box 打包失败
**可能原因**:
1. 路径包含中文或特殊字符
2. 文件被占用（关闭杀毒软件实时保护）
3. 磁盘空间不足

**解决方案**:
- 使用纯英文路径
- 临时禁用杀毒软件
- 检查磁盘空间

#### Q4: UPX 压缩后运行报错
**解决方案**:
- 某些 DLL 不兼容 UPX 压缩
- 排除问题 DLL: 找到报错的 DLL，恢复未压缩版本
- 或完全禁用 UPX 压缩

### Linux 平台

#### Q1: linuxdeployqt 下载失败
**解决方案**:
1. 检查网络连接
2. 使用代理: `export https_proxy=http://proxy:port`
3. 手动下载并放到项目根目录

#### Q2: AppImage 在某些发行版无法运行
**原因**: 
- FUSE 未安装或未启用
- glibc 版本过低

**解决方案**:
```bash
# 安装 FUSE
sudo apt install fuse libfuse2  # Debian/Ubuntu
sudo yum install fuse fuse-libs  # CentOS/RHEL

# 或使用 --appimage-extract 手动解压运行
./LockBoardTester-*.AppImage --appimage-extract
./squashfs-root/AppRun
```

#### Q3: 提示 "Qt version too new"
**原因**: 
linuxdeployqt 对 Qt 版本有限制

**解决方案**:
```bash
# 使用 Qt 5.15 或更早版本
# 或强制运行:
export QMAKE=/path/to/qt5.15/bin/qmake
./build-appimage.sh
```

---

## 工具版本兼容性

| 工具 | Windows | Linux | 备注 |
|------|---------|-------|------|
| Qt 5.12 | ✅ | ✅ | 最低支持版本 |
| Qt 5.15 | ✅ | ✅ | 推荐版本 |
| Qt 6.x | ⚠️ | ⚠️ | 需要修改代码 |
| UPX 3.96 | ✅ | ✅ | |
| UPX 4.0+ | ✅ | ✅ | 推荐版本 |
| Enigma Virtual Box 9.x | ✅ | ❌ | Windows 专用 |
| Enigma Virtual Box 10.x | ✅ | ❌ | 推荐版本 |

---

## 下一步

配置完成后，请参考以下文档：
- [README.md](../README.md) - 项目主文档
- [README_EN.md](../README_EN.md) - English documentation
- [linux-setup.md](linux-setup.md) - Linux 详细配置

或直接开始打包：
```bash
# Windows
scripts\build-windows-single.bat 1.0.0

# Linux
./scripts/build-appimage.sh 1.0.0 --compress

# 跨平台统一脚本
./scripts/build-all.sh --version 1.0.0 --compress
```