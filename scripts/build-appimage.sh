#!/bin/bash

################################################################################
# LockBoardTester AppImage 构建脚本
# 功能：自动构建可移植的 Linux AppImage 包
# 依赖：Qt 5.15+, linuxdeployqt, 可选 UPX
# 用法：./build-appimage.sh [VERSION] [--compress] [--help]
################################################################################

set -e  # 遇到错误立即退出

# 默认版本号
DEFAULT_VERSION="1.0.0"
VERSION="$DEFAULT_VERSION"
USE_COMPRESS=false

# 解析命令行参数
for arg in "$@"; do
    case $arg in
        --compress)
            USE_COMPRESS=true
            ;;
        --help|-h)
            echo "用法: $0 [VERSION] [--compress] [--help]"
            echo ""
            echo "参数："
            echo "  VERSION       指定版本号（默认: $DEFAULT_VERSION）"
            echo "  --compress    使用 UPX 压缩可执行文件和库（需要安装 UPX）"
            echo "  --help, -h    显示此帮助信息"
            echo ""
            echo "示例："
            echo "  $0                    # 使用默认版本 $DEFAULT_VERSION"
            echo "  $0 2.0.1              # 指定版本号"
            echo "  $0 1.5.0 --compress   # 指定版本并启用压缩"
            echo ""
            exit 0
            ;;
        *)
            # 如果是数字开头，认为是版本号
            if [[ $arg =~ ^[0-9] ]]; then
                VERSION="$arg"
            fi
            ;;
    esac
done

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
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

# 检查命令是否存在
check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "未找到命令: $1"
        return 1
    fi
    return 0
}

################################################################################
# 1. 环境检查
################################################################################
print_info "开始检查构建环境..."
print_info "版本号: $VERSION"
if [ "$USE_COMPRESS" = true ]; then
    print_info "压缩模式: 启用"
fi
echo

# 检查 qmake
if ! check_command qmake; then
    print_error "请安装 Qt 开发环境 (qt5-qmake qt5-default)"
    exit 1
fi

# 检查 make
if ! check_command make; then
    print_error "请安装 make 工具"
    exit 1
fi

# 检查可选工具 - UPX
HAS_UPX=false
if command -v upx &> /dev/null; then
    HAS_UPX=true
    UPX_VERSION=$(upx --version 2>&1 | head -n1)
    print_info "检测到 UPX: $UPX_VERSION"
fi

if [ "$USE_COMPRESS" = true ] && [ "$HAS_UPX" = false ]; then
    print_warning "启用了压缩选项但未找到 UPX，将跳过压缩"
    print_warning "安装方法: sudo apt install upx-ucl (Debian/Ubuntu)"
    USE_COMPRESS=false
fi

# 获取 Qt 版本
QT_VERSION=$(qmake -query QT_VERSION)
print_info "检测到 Qt 版本: $QT_VERSION"

# 获取项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

print_info "项目根目录: $PROJECT_ROOT"

################################################################################
# 2. 下载 linuxdeployqt
################################################################################
LINUXDEPLOYQT_URL="https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
LINUXDEPLOYQT="$PROJECT_ROOT/linuxdeployqt-continuous-x86_64.AppImage"

if [ ! -f "$LINUXDEPLOYQT" ]; then
    print_info "下载 linuxdeployqt..."
    if command -v wget &> /dev/null; then
        wget -O "$LINUXDEPLOYQT" "$LINUXDEPLOYQT_URL" || {
            print_error "下载 linuxdeployqt 失败"
            exit 1
        }
    elif command -v curl &> /dev/null; then
        curl -L -o "$LINUXDEPLOYQT" "$LINUXDEPLOYQT_URL" || {
            print_error "下载 linuxdeployqt 失败"
            exit 1
        }
    else
        print_error "需要 wget 或 curl 工具来下载 linuxdeployqt"
        exit 1
    fi
    chmod +x "$LINUXDEPLOYQT"
    print_success "linuxdeployqt 下载完成"
else
    print_info "使用已存在的 linuxdeployqt"
fi

################################################################################
# 3. 清理旧的构建文件
################################################################################
print_info "清理旧的构建文件..."
make clean 2>/dev/null || true
rm -rf AppDir/ LockBoardTester-*.AppImage 2>/dev/null || true
print_success "清理完成"

################################################################################
# 4. 编译项目
################################################################################
print_info "开始编译项目..."
qmake LockBoardTester.pro CONFIG+=release PREFIX=/usr || {
    print_error "qmake 配置失败"
    exit 1
}

make -j$(nproc) || {
    print_error "编译失败"
    exit 1
}
print_success "编译完成"

################################################################################
# 4.5. UPX 压缩可执行文件（可选）
################################################################################
if [ "$USE_COMPRESS" = true ]; then
    print_info "使用 UPX 压缩可执行文件..."
    
    if [ -f "release/LockBoardTester" ]; then
        ORIGINAL_SIZE=$(stat -c%s "release/LockBoardTester")
        upx --best --lzma "release/LockBoardTester" 2>&1 | grep -v "^upx:" || true
        COMPRESSED_SIZE=$(stat -c%s "release/LockBoardTester")
        RATIO=$(echo "scale=1; 100 - ($COMPRESSED_SIZE * 100 / $ORIGINAL_SIZE)" | bc)
        print_success "压缩完成，减少了 ${RATIO}% 大小"
    elif [ -f "LockBoardTester" ]; then
        ORIGINAL_SIZE=$(stat -c%s "LockBoardTester")
        upx --best --lzma "LockBoardTester" 2>&1 | grep -v "^upx:" || true
        COMPRESSED_SIZE=$(stat -c%s "LockBoardTester")
        RATIO=$(echo "scale=1; 100 - ($COMPRESSED_SIZE * 100 / $ORIGINAL_SIZE)" | bc)
        print_success "压缩完成，减少了 ${RATIO}% 大小"
    fi
    echo
fi

################################################################################
# 5. 创建 AppDir 目录结构
################################################################################
print_info "创建 AppDir 目录结构..."
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

# 复制可执行文件
if [ -f "release/LockBoardTester" ]; then
    cp release/LockBoardTester AppDir/usr/bin/
elif [ -f "LockBoardTester" ]; then
    cp LockBoardTester AppDir/usr/bin/
else
    print_error "未找到编译生成的可执行文件"
    exit 1
fi

print_success "AppDir 目录结构创建完成"

################################################################################
# 6. 创建桌面入口文件
################################################################################
print_info "创建桌面入口文件..."
cat > AppDir/usr/share/applications/lockboard-tester.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=LockBoard Tester
Name[zh_CN]=485锁控板测试工具
Comment=RS485 Lock Control Board Communication Tester
Comment[zh_CN]=RS485锁控板通讯测试工具
Exec=LockBoardTester
Icon=lockboard-tester
Categories=Development;Utility;
Terminal=false
Keywords=RS485;Serial;Lock;Testing;
EOF

print_success "桌面入口文件创建完成"

################################################################################
# 7. 创建/复制图标
################################################################################
print_info "处理应用图标..."

# 检查是否有现成的图标
ICON_FOUND=false
for icon_path in "resources/icon.png" "icon.png" "assets/icon.png"; do
    if [ -f "$icon_path" ]; then
        cp "$icon_path" AppDir/usr/share/icons/hicolor/256x256/apps/lockboard-tester.png
        ICON_FOUND=true
        print_success "使用图标: $icon_path"
        break
    fi
done

# 如果没有图标，创建一个简单的占位图标
if [ "$ICON_FOUND" = false ]; then
    print_warning "未找到图标文件，创建默认图标"
    # 使用 ImageMagick 创建简单图标（如果可用）
    if command -v convert &> /dev/null; then
        convert -size 256x256 xc:transparent \
                -fill "#4A90E2" -draw "circle 128,128 128,32" \
                -fill white -pointsize 60 -gravity center -annotate +0+0 "485" \
                AppDir/usr/share/icons/hicolor/256x256/apps/lockboard-tester.png 2>/dev/null || {
            # 如果 ImageMagick 失败，创建一个空白 PNG
            echo -e "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==" | base64 -d > AppDir/usr/share/icons/hicolor/256x256/apps/lockboard-tester.png
        }
    else
        # 创建最简单的 1x1 透明 PNG
        echo -e "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==" | base64 -d > AppDir/usr/share/icons/hicolor/256x256/apps/lockboard-tester.png
    fi
fi

# 复制图标到 AppDir 根目录（linuxdeployqt 需要）
cp AppDir/usr/share/icons/hicolor/256x256/apps/lockboard-tester.png AppDir/lockboard-tester.png

################################################################################
# 8. 使用 linuxdeployqt 打包
################################################################################
print_info "开始打包 AppImage..."
print_info "这可能需要几分钟时间，请耐心等待..."

# 设置 LD_LIBRARY_PATH 包含 Qt 库路径
export LD_LIBRARY_PATH=$(qmake -query QT_INSTALL_LIBS):$LD_LIBRARY_PATH

# 运行 linuxdeployqt
"$LINUXDEPLOYQT" AppDir/usr/share/applications/lockboard-tester.desktop \
    -appimage \
    -bundle-non-qt-libs \
    -verbose=1 || {
    print_error "linuxdeployqt 打包失败"
    print_info "可能的原因："
    print_info "  1. Qt 版本不兼容（建议使用 Qt 5.15）"
    print_info "  2. 缺少必要的系统库"
    print_info "  3. linuxdeployqt 版本问题"
    exit 1
}

################################################################################
# 9. 重命名输出文件
################################################################################
print_info "整理输出文件..."

# 查找生成的 AppImage 文件
APPIMAGE_FILE=$(ls LockBoardTester-*.AppImage 2>/dev/null | head -n1)

if [ -z "$APPIMAGE_FILE" ]; then
    # 尝试其他可能的文件名
    APPIMAGE_FILE=$(ls LockBoard*.AppImage 2>/dev/null | head -n1)
fi

if [ -n "$APPIMAGE_FILE" ]; then
    # 添加版本号和架构信息
    ARCH=$(uname -m)
    NEW_NAME="LockBoardTester-${VERSION}-${ARCH}.AppImage"
    
    mv "$APPIMAGE_FILE" "$NEW_NAME"
    chmod +x "$NEW_NAME"
    
    print_success "================================"
    print_success "AppImage 构建完成！"
    print_success "================================"
    print_info "文件位置: $PROJECT_ROOT/$NEW_NAME"
    print_info "文件大小: $(du -h "$NEW_NAME" | cut -f1)"
    
    if [ "$USE_COMPRESS" = true ]; then
        print_info "已启用 UPX 压缩优化"
    fi
    
    print_info ""
    print_info "运行方式："
    print_info "  chmod +x $NEW_NAME"
    print_info "  ./$NEW_NAME"
    print_info ""
    print_info "注意事项："
    print_info "  1. 首次运行可能需要添加串口访问权限"
    print_info "  2. 运行: sudo usermod -a -G dialout \$USER"
    print_info "  3. 注销并重新登录后生效"
else
    print_error "未找到生成的 AppImage 文件"
    print_info "请检查构建日志中的错误信息"
    exit 1
fi

################################################################################
# 10. 清理临时文件（可选）
################################################################################
read -p "是否清理临时文件？(y/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "清理临时文件..."
    rm -rf AppDir/
    print_success "清理完成"
fi

print_success "所有操作完成！"