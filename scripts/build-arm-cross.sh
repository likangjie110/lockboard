#!/bin/bash

################################################################################
# LockBoardTester ARM 交叉编译脚本
# 功能：使用交叉编译工具链为 ARM 平台编译
# 支持架构：armv7l (32位), aarch64 (64位)
# 用法：./build-arm-cross.sh [ARCH] [VERSION] [--static] [--help]
################################################################################

set -e

# 默认参数
DEFAULT_VERSION="1.0.0"
VERSION="$DEFAULT_VERSION"
TARGET_ARCH="armv7l"  # 默认 ARM 32位
STATIC_BUILD=false

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        armv7l|armv7|arm32)
            TARGET_ARCH="armv7l"
            shift
            ;;
        aarch64|arm64)
            TARGET_ARCH="aarch64"
            shift
            ;;
        --static)
            STATIC_BUILD=true
            shift
            ;;
        --help|-h)
            echo "用法: $0 [ARCH] [VERSION] [--static] [--help]"
            echo ""
            echo "架构选项："
            echo "  armv7l, armv7, arm32    ARM 32位（默认）"
            echo "  aarch64, arm64          ARM 64位"
            echo ""
            echo "其他参数："
            echo "  VERSION                 指定版本号（默认: $DEFAULT_VERSION）"
            echo "  --static                静态链接（需要静态版 Qt）"
            echo "  --help, -h              显示此帮助信息"
            echo ""
            echo "示例："
            echo "  $0                           # ARM 32位，默认版本"
            echo "  $0 aarch64 1.0.1             # ARM 64位，指定版本"
            echo "  $0 armv7l 1.0.0 --static     # ARM 32位静态编译"
            echo ""
            echo "环境变量（可选配置）："
            echo "  QT_CROSS_PATH              Qt 交叉编译版本路径"
            echo "  CROSS_COMPILE_PREFIX       交叉编译工具链前缀"
            echo "  SYSROOT                    目标系统根目录"
            echo ""
            exit 0
            ;;
        *)
            # 如果是数字开头，认为是版本号
            if [[ $1 =~ ^[0-9] ]]; then
                VERSION="$1"
            fi
            shift
            ;;
    esac
done

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "未找到命令: $1"
        return 1
    fi
    return 0
}

################################################################################
# 1. 环境检查和配置
################################################################################
print_info "=========================================="
print_info "LockBoardTester ARM 交叉编译"
print_info "=========================================="
print_info "目标架构: $TARGET_ARCH"
print_info "版本号: $VERSION"
print_info "静态编译: $STATIC_BUILD"
print_info ""

# 获取项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

print_info "项目根目录: $PROJECT_ROOT"

# 根据架构设置工具链前缀
if [ "$TARGET_ARCH" = "armv7l" ]; then
    DEFAULT_CROSS_PREFIX="arm-linux-gnueabihf-"
    DEFAULT_QT_ARCH="armv7"
else
    DEFAULT_CROSS_PREFIX="aarch64-linux-gnu-"
    DEFAULT_QT_ARCH="aarch64"
fi

# 设置交叉编译工具链前缀（可通过环境变量覆盖）
CROSS_PREFIX="${CROSS_COMPILE_PREFIX:-$DEFAULT_CROSS_PREFIX}"

print_info "交叉编译工具链前缀: $CROSS_PREFIX"

################################################################################
# 2. 检查交叉编译工具链
################################################################################
print_info "检查交叉编译工具链..."

if ! command -v "${CROSS_PREFIX}gcc" &> /dev/null; then
    print_error "未找到交叉编译器: ${CROSS_PREFIX}gcc"
    echo ""
    echo "请安装交叉编译工具链："
    if [ "$TARGET_ARCH" = "armv7l" ]; then
        echo "  Ubuntu/Debian: sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf"
        echo "  Fedora:        sudo dnf install gcc-arm-linux-gnu gcc-c++-arm-linux-gnu"
    else
        echo "  Ubuntu/Debian: sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"
        echo "  Fedora:        sudo dnf install gcc-aarch64-linux-gnu gcc-c++-aarch64-linux-gnu"
    fi
    echo ""
    exit 1
fi

# 显示工具链版本
GCC_VERSION=$(${CROSS_PREFIX}gcc --version | head -n1)
print_success "交叉编译器: $GCC_VERSION"

################################################################################
# 3. 检查 Qt 交叉编译版本
################################################################################
print_info "检查 Qt 交叉编译版本..."

# 尝试查找 Qt 交叉编译版本
if [ -n "$QT_CROSS_PATH" ]; then
    QT_PATH="$QT_CROSS_PATH"
elif [ -d "/opt/qt5-$DEFAULT_QT_ARCH" ]; then
    QT_PATH="/opt/qt5-$DEFAULT_QT_ARCH"
elif [ -d "$HOME/qt5-$DEFAULT_QT_ARCH" ]; then
    QT_PATH="$HOME/qt5-$DEFAULT_QT_ARCH"
else
    print_warning "未找到 Qt 交叉编译版本"
    print_info "将使用主机 Qt + 交叉编译（可能导致库依赖问题）"
    print_info ""
    print_info "建议安装 Qt 交叉编译版本："
    print_info "  1. 下载 Qt 源码: https://download.qt.io/archive/qt/5.15/5.15.2/single/"
    print_info "  2. 配置交叉编译:"
    print_info "     ./configure -prefix /opt/qt5-$DEFAULT_QT_ARCH \\"
    print_info "                 -device linux-rasp-pi3-g++ \\"
    print_info "                 -device-option CROSS_COMPILE=$CROSS_PREFIX \\"
    print_info "                 -sysroot \$SYSROOT \\"
    print_info "                 -opensource -confirm-license \\"
    print_info "                 -release -optimized-qmake \\"
    print_info "                 -nomake examples -nomake tests"
    print_info "  3. 编译安装: make -j\$(nproc) && sudo make install"
    print_info ""
    
    # 检查是否有 qmake
    if ! check_command qmake; then
        print_error "未找到 qmake，无法继续"
        exit 1
    fi
    QT_PATH=$(qmake -query QT_INSTALL_PREFIX)
    print_warning "使用主机 Qt 路径: $QT_PATH"
fi

if [ -d "$QT_PATH" ]; then
    print_success "Qt 路径: $QT_PATH"
    QMAKE_BIN="$QT_PATH/bin/qmake"
    
    if [ ! -f "$QMAKE_BIN" ]; then
        QMAKE_BIN="qmake"
    fi
else
    print_warning "Qt 路径不存在: $QT_PATH"
    QMAKE_BIN="qmake"
fi

################################################################################
# 4. 创建交叉编译配置文件
################################################################################
print_info "创建交叉编译配置..."

CROSS_SPEC_DIR="$PROJECT_ROOT/mkspecs/linux-arm-gnueabi-g++"
mkdir -p "$CROSS_SPEC_DIR"

cat > "$CROSS_SPEC_DIR/qmake.conf" << EOF
#
# qmake configuration for ARM cross-compilation
#

MAKEFILE_GENERATOR      = UNIX
CONFIG                 += incremental
QMAKE_INCREMENTAL_STYLE = sublib

include(../../common/linux.conf)
include(../../common/gcc-base-unix.conf)
include(../../common/g++-unix.conf)

# Compiler settings
QMAKE_CC                = ${CROSS_PREFIX}gcc
QMAKE_CXX               = ${CROSS_PREFIX}g++
QMAKE_LINK              = ${CROSS_PREFIX}g++
QMAKE_LINK_SHLIB        = ${CROSS_PREFIX}g++

# Binutils
QMAKE_AR                = ${CROSS_PREFIX}ar cqs
QMAKE_OBJCOPY           = ${CROSS_PREFIX}objcopy
QMAKE_NM                = ${CROSS_PREFIX}nm -P
QMAKE_STRIP             = ${CROSS_PREFIX}strip

EOF

if [ -n "$SYSROOT" ] && [ -d "$SYSROOT" ]; then
    echo "QMAKE_CFLAGS            += --sysroot=\$\$SYSROOT" >> "$CROSS_SPEC_DIR/qmake.conf"
    echo "QMAKE_CXXFLAGS          += --sysroot=\$\$SYSROOT" >> "$CROSS_SPEC_DIR/qmake.conf"
    echo "QMAKE_LFLAGS            += --sysroot=\$\$SYSROOT" >> "$CROSS_SPEC_DIR/qmake.conf"
    print_info "使用 SYSROOT: $SYSROOT"
fi

cat >> "$CROSS_SPEC_DIR/qmake.conf" << 'EOF'

load(qt_config)
EOF

cat > "$CROSS_SPEC_DIR/qplatformdefs.h" << 'EOF'
#include "../../linux-g++/qplatformdefs.h"
EOF

print_success "交叉编译配置创建完成"

################################################################################
# 5. 清理旧构建
################################################################################
print_info "清理旧构建文件..."
make clean 2>/dev/null || true
rm -rf build-arm-$TARGET_ARCH/ 2>/dev/null || true
print_success "清理完成"

################################################################################
# 6. 配置和编译
################################################################################
print_info "开始交叉编译..."

BUILD_DIR="build-arm-$TARGET_ARCH"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 构建 qmake 命令
QMAKE_CMD="$QMAKE_BIN"
QMAKE_ARGS="-spec ../mkspecs/linux-arm-gnueabi-g++ CONFIG+=release"

if [ "$STATIC_BUILD" = true ]; then
    QMAKE_ARGS="$QMAKE_ARGS CONFIG+=static"
    print_info "启用静态链接"
fi

print_info "执行: $QMAKE_CMD ../LockBoardTester.pro $QMAKE_ARGS"
$QMAKE_CMD ../LockBoardTester.pro $QMAKE_ARGS || {
    print_error "qmake 配置失败"
    exit 1
}

print_info "编译中（使用 $(nproc) 个并行任务）..."
make -j$(nproc) || {
    print_error "编译失败"
    exit 1
}

print_success "编译完成"

################################################################################
# 7. 检查生成的二进制文件
################################################################################
print_info "检查生成的二进制文件..."

if [ -f "LockBoardTester" ]; then
    BINARY="LockBoardTester"
elif [ -f "release/LockBoardTester" ]; then
    BINARY="release/LockBoardTester"
else
    print_error "未找到编译生成的可执行文件"
    exit 1
fi

# 检查架构
FILE_INFO=$(file "$BINARY")
print_info "二进制文件信息:"
echo "  $FILE_INFO"

# 检查动态链接库依赖
print_info "依赖库检查:"
${CROSS_PREFIX}readelf -d "$BINARY" | grep "NEEDED" || true

################################################################################
# 8. 创建发布包
################################################################################
print_info "创建发布包..."

cd "$PROJECT_ROOT"
DIST_DIR="dist/lockboard-$VERSION-linux-$TARGET_ARCH"
mkdir -p "$DIST_DIR"

# 复制可执行文件
cp "$BUILD_DIR/$BINARY" "$DIST_DIR/"

# 创建启动脚本
cat > "$DIST_DIR/run.sh" << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="$SCRIPT_DIR/lib:$LD_LIBRARY_PATH"
"$SCRIPT_DIR/LockBoardTester" "$@"
EOF
chmod +x "$DIST_DIR/run.sh"

# 创建 README
cat > "$DIST_DIR/README.txt" << EOF
LockBoardTester for ARM Linux
==============================

版本: $VERSION
架构: $TARGET_ARCH
编译时间: $(date)

安装说明：
1. 解压到目标设备
2. 确保串口权限：sudo usermod -a -G dialout \$USER
3. 运行：./run.sh

依赖库：
- Qt 5.15+ (Core, Gui, Widgets, SerialPort)
- 如果缺少库，请在目标设备安装：
  sudo apt install libqt5core5a libqt5gui5 libqt5widgets5 libqt5serialport5

如果是静态编译版本，无需安装 Qt。
EOF

# 打包
cd dist
TAR_NAME="LockBoardTester-$VERSION-linux-$TARGET_ARCH.tar.gz"
tar czf "$TAR_NAME" "lockboard-$VERSION-linux-$TARGET_ARCH"

print_success "=========================================="
print_success "ARM 交叉编译完成！"
print_success "=========================================="
print_info "输出目录: $PROJECT_ROOT/dist/lockboard-$VERSION-linux-$TARGET_ARCH"
print_info "发布包: $PROJECT_ROOT/dist/$TAR_NAME"
print_info "文件大小: $(du -h "$TAR_NAME" | cut -f1)"
print_info ""
print_info "部署到目标设备："
print_info "  scp $TAR_NAME user@target-device:/tmp/"
print_info "  ssh user@target-device"
print_info "  cd /tmp && tar xzf $TAR_NAME"
print_info "  cd lockboard-$VERSION-linux-$TARGET_ARCH"
print_info "  ./run.sh"
print_info ""

if [ "$STATIC_BUILD" = false ]; then
    print_warning "注意：这是动态链接版本，目标设备需要安装 Qt 运行时库"
    print_info "或者使用 --static 选项创建静态版本（需要静态版 Qt）"
fi