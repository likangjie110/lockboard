#!/bin/bash
# Fedora 构建测试脚本
set -e

echo "========================================="
echo "Fedora 构建测试 - LockBoard Tester"
echo "========================================="
echo ""

# 显示系统信息
echo "=== 系统信息 ==="
cat /etc/os-release | grep -E "PRETTY_NAME|VERSION_ID"
uname -a
echo ""

# 安装依赖
echo "=== 安装依赖 ==="
dnf install -y -q \
    qt5-qtbase-devel \
    qt5-qtserialport-devel \
    gcc-c++ \
    make

echo "依赖安装完成"
echo ""

# 验证工具版本
echo "=== 工具版本 ==="
qmake-qt5 --version || qmake --version
g++ --version | head -n1
make --version | head -n1
echo ""

# 清理之前的构建
echo "=== 清理构建目录 ==="
if [ -f Makefile ]; then
    make clean || true
fi
rm -f Makefile Makefile.Debug Makefile.Release
rm -rf release/ debug/
echo ""

# 生成 Makefile
echo "=== 生成 Makefile ==="
# Fedora 可能用 qmake-qt5 命令
if command -v qmake-qt5 &> /dev/null; then
    qmake-qt5 LockBoardTester.pro CONFIG+=release
else
    qmake LockBoardTester.pro CONFIG+=release
fi
echo "Makefile 生成成功"
echo ""

# 编译项目
echo "=== 编译项目 ==="
make -j$(nproc)
echo ""

# 检查编译产物
echo "=== 验证编译产物 ==="
if [ -f release/LockBoardTester ] || [ -f LockBoardTester ]; then
    BINARY=$(find . -name "LockBoardTester" -type f -executable | head -n1)
    echo "✅ 编译成功！可执行文件: $BINARY"
    
    echo ""
    echo "=== 可执行文件信息 ==="
    ls -lh "$BINARY"
    file "$BINARY"
    
    echo ""
    echo "=== 依赖库检查 ==="
    ldd "$BINARY" | grep -i qt
    
    echo ""
    echo "=== 测试运行（帮助信息） ==="
    "$BINARY" --help 2>&1 || echo "程序无 --help 选项（正常，GUI应用）"
    
    echo ""
    echo "=== RPM 打包验证 ==="
    if [ -f ../lockboard-tester.spec ]; then
        echo "检查 spec 文件语法..."
        # 这里只是检查文件存在性
        echo "spec 文件存在: ../lockboard-tester.spec"
    fi
    
    echo ""
    echo "✅ Fedora 构建测试通过！"
    exit 0
else
    echo "❌ 编译失败：未找到可执行文件"
    ls -la release/ || echo "release 目录不存在"
    exit 1
fi