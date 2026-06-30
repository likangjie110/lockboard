#!/bin/bash

################################################################################
# LockBoardTester 跨平台统一构建脚本
# 功能：自动检测操作系统并调用对应平台的打包脚本
# 支持：Windows (Git Bash/MSYS2/WSL), Linux
################################################################################

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 打印消息函数
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

# 显示帮助信息
show_help() {
    cat << EOF
LockBoardTester 跨平台构建脚本

用法: $0 [选项]

选项:
  --version VERSION    指定版本号（默认: 1.0.0）
  --compress           启用 UPX 压缩（需要安装 UPX）
  --clean              构建前清理所有旧文件
  --help, -h           显示此帮助信息

示例:
  $0                              # 使用默认设置构建
  $0 --version 2.0.1              # 指定版本号
  $0 --version 1.5.0 --compress   # 指定版本并启用压缩
  $0 --clean --compress           # 清理后构建并压缩

支持的平台:
  - Linux (原生)
  - Windows (通过 Git Bash/MSYS2/WSL)

EOF
    exit 0
}

################################################################################
# 解析命令行参数
################################################################################
VERSION="1.0.0"
USE_COMPRESS=""
DO_CLEAN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --compress)
            USE_COMPRESS="--compress"
            shift
            ;;
        --clean)
            DO_CLEAN=true
            shift
            ;;
        --help|-h)
            show_help
            ;;
        *)
            print_error "未知选项: $1"
            echo "使用 --help 查看帮助信息"
            exit 1
            ;;
    esac
done

################################################################################
# 检测操作系统
################################################################################
print_info "检测操作系统..."

OS_TYPE=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_TYPE="linux"
    print_info "检测到 Linux 系统"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
    OS_TYPE="windows"
    print_info "检测到 Windows 系统 (Git Bash/MSYS2)"
elif grep -qi microsoft /proc/version 2>/dev/null; then
    OS_TYPE="wsl"
    print_info "检测到 WSL 环境"
    print_warning "WSL 环境建议直接使用 Linux 原生构建"
    print_warning "如需构建 Windows 版本，请在 Windows 上运行 build-windows-single.bat"
else
    print_error "无法识别的操作系统: $OSTYPE"
    exit 1
fi

################################################################################
# 获取脚本目录
################################################################################
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

print_info "项目根目录: $PROJECT_ROOT"
print_info "版本号: $VERSION"

################################################################################
# 清理操作
################################################################################
if [ "$DO_CLEAN" = true ]; then
    print_info "清理旧构建文件..."
    
    # 清理通用文件
    rm -rf AppDir/ *.AppImage 2>/dev/null || true
    rm -rf release/ dist/ 2>/dev/null || true
    rm -f Makefile Makefile.Debug Makefile.Release 2>/dev/null || true
    make clean 2>/dev/null || true
    
    print_success "清理完成"
    echo
fi

################################################################################
# 调用平台特定的构建脚本
################################################################################
print_info "开始构建..."
echo

case $OS_TYPE in
    linux)
        # Linux 平台使用 AppImage 脚本
        SCRIPT_PATH="$SCRIPT_DIR/build-appimage.sh"
        
        if [ ! -f "$SCRIPT_PATH" ]; then
            print_error "未找到 Linux 构建脚本: $SCRIPT_PATH"
            exit 1
        fi
        
        chmod +x "$SCRIPT_PATH"
        "$SCRIPT_PATH" "$VERSION" $USE_COMPRESS
        
        if [ $? -eq 0 ]; then
            print_success "================================"
            print_success "Linux 打包完成！"
            print_success "================================"
        else
            print_error "Linux 打包失败"
            exit 1
        fi
        ;;
        
    windows)
        # Windows 平台使用批处理脚本
        SCRIPT_PATH="$SCRIPT_DIR/build-windows-single.bat"
        
        if [ ! -f "$SCRIPT_PATH" ]; then
            print_error "未找到 Windows 构建脚本: $SCRIPT_PATH"
            exit 1
        fi
        
        # 在 Git Bash/MSYS2 中调用批处理脚本
        print_info "调用 Windows 构建脚本..."
        
        if [ -n "$USE_COMPRESS" ]; then
            print_warning "Windows 构建脚本会自动检测并使用 UPX（如果可用）"
        fi
        
        cmd.exe /c "$(cygpath -w "$SCRIPT_PATH")" "$VERSION"
        
        if [ $? -eq 0 ]; then
            print_success "================================"
            print_success "Windows 打包完成！"
            print_success "================================"
        else
            print_error "Windows 打包失败"
            exit 1
        fi
        ;;
        
    wsl)
        # WSL 环境按 Linux 处理，但给出警告
        print_warning "在 WSL 中构建 Linux 版本..."
        
        SCRIPT_PATH="$SCRIPT_DIR/build-appimage.sh"
        
        if [ ! -f "$SCRIPT_PATH" ]; then
            print_error "未找到 Linux 构建脚本: $SCRIPT_PATH"
            exit 1
        fi
        
        chmod +x "$SCRIPT_PATH"
        "$SCRIPT_PATH" "$VERSION" $USE_COMPRESS
        
        if [ $? -eq 0 ]; then
            print_success "================================"
            print_success "Linux (WSL) 打包完成！"
            print_success "================================"
            print_info "注意: 此为 Linux 版本，仅能在 WSL/Linux 环境运行"
        else
            print_error "Linux (WSL) 打包失败"
            exit 1
        fi
        ;;
esac

echo
print_success "所有操作完成！"
print_info "详细文档请参考: docs/packaging-setup.md"

exit 0