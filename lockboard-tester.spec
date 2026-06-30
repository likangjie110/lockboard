Name:           lockboard-tester
Version:        1.0.0
Release:        1%{?dist}
Summary:        RS485锁控板通讯测试工具
Summary(zh_CN): RS485锁控板通讯测试工具

License:        MIT
URL:            https://github.com/yourorg/lockboard-tester
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  qt5-qtbase-devel >= 5.15
BuildRequires:  qt5-qtserialport-devel >= 5.15
BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  make

Requires:       qt5-qtbase >= 5.15
Requires:       qt5-qtserialport >= 5.15
Recommends:     wqy-microhei-fonts
Recommends:     google-noto-sans-cjk-fonts

%description
LockBoardTester 是一个基于 Qt 5.15 开发的 RS485 锁控板通讯测试工具，
支持 12 路锁控制和完整的协议命令验证。

主要功能：
- 支持 9 种协议命令（版本查询、单锁控制、批量控制等）
- 12 路锁状态实时可视化显示
- 完整的调试控制台（十六进制日志）
- 自动串口扫描和连接管理
- 支持多板卡级联（地址 1-15）

适用场景：
- 锁控板硬件测试和调试
- RS485 协议开发和验证
- 生产线质量检测
- 现场安装和维护

%description -l zh_CN
LockBoardTester 是一个基于 Qt 5.15 开发的 RS485 锁控板通讯测试工具，
支持 12 路锁控制和完整的协议命令验证。

主要功能：
- 支持 9 种协议命令（版本查询、单锁控制、批量控制等）
- 12 路锁状态实时可视化显示
- 完整的调试控制台（十六进制日志）
- 自动串口扫描和连接管理
- 支持多板卡级联（地址 1-15）


%prep
%setup -q


%build
# 设置 Qt5 环境
export PATH=%{_qt5_bindir}:$PATH

# 生成 Makefile
%{qmake_qt5} LockBoardTester.pro \
    CONFIG+=release \
    PREFIX=%{_prefix}

# 编译
make %{?_smp_mflags}


%install
# 创建目录结构
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/256x256/apps
mkdir -p %{buildroot}%{_datadir}/pixmaps
mkdir -p %{buildroot}%{_mandir}/man1
mkdir -p %{buildroot}%{_docdir}/%{name}

# 安装可执行文件
install -m 0755 release/LockBoardTester %{buildroot}%{_bindir}/lockboard-tester

# 创建桌面入口文件
cat > %{buildroot}%{_datadir}/applications/lockboard-tester.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=LockBoard Tester
Name[zh_CN]=485锁控板测试工具
Comment=RS485 Lock Control Board Communication Tester
Comment[zh_CN]=RS485锁控板通讯测试工具
Exec=lockboard-tester
Icon=lockboard-tester
Categories=Development;Utility;Electronics;
Terminal=false
Keywords=RS485;Serial;Lock;Testing;
EOF

# 安装图标（如果存在）
if [ -f resources/icon.png ]; then
    install -m 0644 resources/icon.png \
        %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/lockboard-tester.png
    install -m 0644 resources/icon.png \
        %{buildroot}%{_datadir}/pixmaps/lockboard-tester.png
fi

# 安装文档
install -m 0644 README.md %{buildroot}%{_docdir}/%{name}/
if [ -f docs/troubleshooting-linux.md ]; then
    install -m 0644 docs/troubleshooting-linux.md %{buildroot}%{_docdir}/%{name}/
fi

# 验证桌面文件
desktop-file-validate %{buildroot}%{_datadir}/applications/lockboard-tester.desktop


%post
# 更新桌面数据库
/usr/bin/update-desktop-database &> /dev/null || :

# 更新图标缓存
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

# 添加用户提示
cat << 'EOMSG'

================================================================================
LockBoardTester 安装完成！

首次使用前，请将当前用户添加到 dialout 组以获取串口访问权限：

    sudo usermod -a -G dialout $USER

然后注销并重新登录，或者运行：

    newgrp dialout

启动程序：

    lockboard-tester

或从应用程序菜单中查找"485锁控板测试工具"。

详细使用说明请查看：
    %{_docdir}/%{name}/README.md

故障排除指南：
    %{_docdir}/%{name}/troubleshooting-linux.md

================================================================================
EOMSG


%postun
# 更新桌面数据库
/usr/bin/update-desktop-database &> /dev/null || :

# 更新图标缓存
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi


%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :


%files
%license LICENSE
%doc README.md
%doc docs/troubleshooting-linux.md
%{_bindir}/lockboard-tester
%{_datadir}/applications/lockboard-tester.desktop
%{_datadir}/icons/hicolor/256x256/apps/lockboard-tester.png
%{_datadir}/pixmaps/lockboard-tester.png
%{_docdir}/%{name}/


%changelog
* Tue Jun 30 2026 Development Team <dev@example.com> - 1.0.0-1
- 初始版本发布
- 支持全部 9 种协议命令
- 12 路锁状态可视化
- 调试控制台日志功能
- 支持多板卡级联（地址 1-15）

* Mon Jun 29 2026 Development Team <dev@example.com> - 0.9.0-1
- Beta 测试版本
- 实现基础功能
- 完成协议封装

* Sun Jun 28 2026 Development Team <dev@example.com> - 0.1.0-1
- 初始开发版本