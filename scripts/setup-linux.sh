#!/bin/bash
# LockBoard Tester Linux 自动化安装脚本
# 支持 Ubuntu/Debian、Fedora/RHEL、Arch Linux、openSUSE

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印函数
print_info() {
    echo -e "${BLUE}[信息]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[成功]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[警告]${NC} $1"
}

print_error() {
    echo -e "${RED}[错误]${NC} $1"
}

print_step() {
    echo -e "\n${GREEN}===${NC} $1 ${GREEN}===${NC}\n"
}

# 检查是否以 root 权限运行
check_root() {
    if [ "$EUID" -ne 0 ]; then 
        print_error "请使用 sudo 运行此脚本"
        print_info "示例: sudo bash $0"
        exit 1
    fi
}

# 检测 Linux 发行版
detect_distro() {
    print_step "检测 Linux 发行版"
    
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_VERSION=$VERSION_ID
        print_info "检测到发行版: $NAME $VERSION"
    else
        print_error "无法检测 Linux 发行版"
        exit 1
    fi
}

# 安装 Qt 依赖
install_qt_deps() {
    print_step "安装 Qt 开发依赖"
    
    case "$DISTRO" in
        ubuntu|debian|linuxmint|pop)
            print_info "使用 apt 包管理器安装..."
            apt update
            apt install -y \
                qt5-qmake \
                qtbase5-dev \
                qtbase5-dev-tools \
                libqt5serialport5-dev \
                build-essential
            print_success "Qt 依赖安装完成"
            ;;
            
        fedora)
            print_info "使用 dnf 包管理器安装..."
            dnf install -y \
                qt5-qtbase-devel \
                qt5-qtserialport-devel \
                gcc-c++ \
                make
            print_success "Qt 依赖安装完成"
            ;;
            
        rhel|centos|rocky|almalinux)
            print_info "使用 yum 包管理器安装..."
            # 确保 EPEL 仓库可用
            if ! rpm -q epel-release > /dev/null 2>&1; then
                print_info "安装 EPEL 仓库..."
                yum install -y epel-release
            fi
            yum install -y \
                qt5-qtbase-devel \
                qt5-qtserialport-devel \
                gcc-c++ \
                make
            print_success "Qt 依赖安装完成"
            ;;
            
        arch|manjaro|endeavouros)
            print_info "使用 pacman 包管理器安装..."
            pacman -Sy --noconfirm \
                qt5-base \
                qt5-serialport \
                base-devel
            print_success "Qt 依赖安装完成"
            ;;
            
        opensuse*|sles)
            print_info "使用 zypper 包管理器安装..."
            zypper install -y \
                libqt5-qtbase-devel \
                libqt5-qtserialport-devel \
                gcc-c++ \
                make
            print_success "Qt 依赖安装完成"
            ;;
            
        *)
            print_warning "未识别的发行版: $DISTRO"
            print_info "请手动安装以下依赖："
            print_info "  - Qt 5.15+ 开发包"
            print_info "  - Qt SerialPort 模块"
            print_info "  - C++ 编译器 (gcc/g++)"
            print_info "  - make 工具"
            read -p "按回车键继续，或按 Ctrl+C 退出..." 
            ;;
    esac
}

# 配置串口权限
setup_serial_permissions() {
    print_step "配置串口访问权限"
    
    # 确定使用的组名
    if getent group dialout > /dev/null 2>&1; then
        SERIAL_GROUP="dialout"
    elif getent group uucp > /dev/null 2>&1; then
        SERIAL_GROUP="uucp"
    else
        print_warning "未找到 dialout 或 uucp 组，跳过用户组配置"
        return
    fi
    
    print_info "串口设备组: $SERIAL_GROUP"
    
    # 获取实际运行脚本的用户（即使用 sudo 的原始用户）
    if [ -n "$SUDO_USER" ]; then
        ACTUAL_USER="$SUDO_USER"
    else
        ACTUAL_USER=$(logname 2>/dev/null || echo "$USER")
    fi
    
    print_info "为用户 $ACTUAL_USER 配置权限..."
    
    # 检查用户是否已在组中
    if groups "$ACTUAL_USER" | grep -q "\b$SERIAL_GROUP\b"; then
        print_info "用户 $ACTUAL_USER 已在 $SERIAL_GROUP 组中"
    else
        usermod -aG "$SERIAL_GROUP" "$ACTUAL_USER"
        print_success "已将用户 $ACTUAL_USER 添加到 $SERIAL_GROUP 组"
        print_warning "需要注销后重新登录才能生效，或运行: newgrp $SERIAL_GROUP"
    fi
}

# 安装 udev 规则
install_udev_rules() {
    print_step "安装 udev 规则"
    
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    UDEV_RULE_FILE="$SCRIPT_DIR/99-lockboard.rules"
    
    if [ -f "$UDEV_RULE_FILE" ]; then
        print_info "复制 udev 规则文件到 /etc/udev/rules.d/ ..."
        cp "$UDEV_RULE_FILE" /etc/udev/rules.d/
        
        print_info "重新加载 udev 规则..."
        udevadm control --reload-rules
        udevadm trigger
        
        print_success "udev 规则安装完成"
        print_info "USB 转串口设备将自动获得访问权限"
    else
        print_warning "未找到 udev 规则文件: $UDEV_RULE_FILE"
        print_info "跳过 udev 规则安装"
    fi
}

# 验证安装
verify_installation() {
    print_step "验证安装"
    
    # 检查 qmake
    if command -v qmake > /dev/null 2>&1; then
        QMAKE_VERSION=$(qmake --version | grep "Using Qt version" | awk '{print $4}')
        print_success "qmake 已安装: $QMAKE_VERSION"
    else
        print_error "qmake 未找到"
        return 1
    fi
    
    # 检查编译器
    if command -v g++ > /dev/null 2>&1; then
        GCC_VERSION=$(g++ --version | head -n1 | awk '{print $3}')
        print_success "g++ 已安装: $GCC_VERSION"
    else
        print_error "g++ 未找到"
        return 1
    fi
    
    # 检查 make
    if command -v make > /dev/null 2>&1; then
        print_success "make 已安装"
    else
        print_error "make 未找到"
        return 1
    fi
    
    print_success "所有依赖验证通过"
}

# 编译项目（可选）
compile_project() {
    print_step "编译项目"
    
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
    
    if [ -f "$PROJECT_DIR/LockBoardTester.pro" ]; then
        read -p "是否现在编译项目？[Y/n] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
            print_info "进入项目目录: $PROJECT_DIR"
            cd "$PROJECT_DIR"
            
            print_info "生成 Makefile..."
            qmake LockBoardTester.pro
            
            print_info "开始编译（使用 $(nproc) 核心）..."
            make -j$(nproc)
            
            print_success "编译完成"
            print_info "可执行文件位置: $PROJECT_DIR/LockBoardTester"
        else
            print_info "跳过编译"
            print_info "稍后可以手动编译:"
            print_info "  cd $PROJECT_DIR"
            print_info "  qmake LockBoardTester.pro"
            print_info "  make -j\$(nproc)"
        fi
    else
        print_warning "未找到项目文件，跳过编译"
    fi
}

# 显示完成信息
show_completion() {
    print_step "安装完成"
    
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    print_success "LockBoard Tester Linux 环境配置完成！"
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    echo -e "\n${BLUE}后续步骤：${NC}"
    echo "1. 注销并重新登录以使串口权限生效"
    echo "   或运行: newgrp dialout (或 newgrp uucp)"
    echo ""
    echo "2. 编译项目（如果还未编译）："
    echo "   cd $(dirname "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)")"
    echo "   qmake LockBoardTester.pro"
    echo "   make -j\$(nproc)"
    echo ""
    echo "3. 运行程序："
    echo "   ./LockBoardTester"
    echo ""
    echo "4. 查看详细文档："
    echo "   cat docs/linux-setup.md"
    echo ""
    print_info "如遇问题，请参考文档中的"常见问题"章节"
}

# 主流程
main() {
    echo -e "${BLUE}"
    echo "╔════════════════════════════════════════════════╗"
    echo "║  LockBoard Tester - Linux 自动化安装脚本      ║"
    echo "║  版本: 1.0.0                                   ║"
    echo "╚════════════════════════════════════════════════╝"
    echo -e "${NC}\n"
    
    check_root
    detect_distro
    install_qt_deps
    setup_serial_permissions
    install_udev_rules
    verify_installation
    compile_project
    show_completion
}

# 运行主流程
main "$@"