@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

::===============================================================================
:: LockBoardTester Windows 单文件打包脚本
:: 功能：编译并打包为单个可执行文件
:: 依赖：Qt 5.15+, MinGW/MSVC, 可选 UPX 和 Enigma Virtual Box
::===============================================================================

:: 颜色代码（Windows 10+）
set "C_INFO=[94m"
set "C_SUCCESS=[92m"
set "C_WARNING=[93m"
set "C_ERROR=[91m"
set "C_RESET=[0m"

:: 版本号参数
set VERSION=1.0.0
if not "%~1"=="" set VERSION=%~1

:: 打印带颜色的消息
:print_info
echo %C_INFO%[INFO]%C_RESET% %~1
goto :eof

:print_success
echo %C_SUCCESS%[SUCCESS]%C_RESET% %~1
goto :eof

:print_warning
echo %C_WARNING%[WARNING]%C_RESET% %~1
goto :eof

:print_error
echo %C_ERROR%[ERROR]%C_RESET% %~1
goto :eof

::===============================================================================
:: 主流程开始
::===============================================================================
call :print_info "LockBoardTester Windows 打包脚本"
call :print_info "版本号: %VERSION%"
echo.

::===============================================================================
:: 1. 环境检查
::===============================================================================
call :print_info "检查构建环境..."

:: 检查 qmake
where qmake >nul 2>&1
if errorlevel 1 (
    call :print_error "未找到 qmake，请安装 Qt 并添加到 PATH"
    exit /b 1
)

:: 获取 Qt 版本
for /f "tokens=*" %%i in ('qmake -query QT_VERSION') do set QT_VERSION=%%i
call :print_info "检测到 Qt 版本: %QT_VERSION%"

:: 检查 windeployqt
where windeployqt >nul 2>&1
if errorlevel 1 (
    call :print_error "未找到 windeployqt，请确保 Qt bin 目录在 PATH 中"
    exit /b 1
)

:: 检查编译器
set COMPILER_FOUND=0
where mingw32-make >nul 2>&1
if not errorlevel 1 (
    set MAKE_CMD=mingw32-make
    set COMPILER_FOUND=1
    call :print_info "检测到 MinGW 编译器"
)

if %COMPILER_FOUND%==0 (
    where nmake >nul 2>&1
    if not errorlevel 1 (
        set MAKE_CMD=nmake
        set COMPILER_FOUND=1
        call :print_info "检测到 MSVC 编译器"
    )
)

if %COMPILER_FOUND%==0 (
    call :print_error "未找到 MinGW 或 MSVC 编译器"
    exit /b 1
)

:: 检查可选工具 - UPX
set HAS_UPX=0
where upx >nul 2>&1
if not errorlevel 1 (
    set HAS_UPX=1
    for /f "tokens=*" %%i in ('upx --version 2^>^&1 ^| findstr /r "^upx"') do (
        call :print_info "检测到 UPX: %%i"
    )
)

:: 检查可选工具 - Enigma Virtual Box
set HAS_ENIGMA=0
set ENIGMA_CLI=
if exist "C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe" (
    set HAS_ENIGMA=1
    set "ENIGMA_CLI=C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe"
    call :print_info "检测到 Enigma Virtual Box"
)
if exist "C:\Program Files\Enigma Virtual Box\enigmavbconsole.exe" (
    set HAS_ENIGMA=1
    set "ENIGMA_CLI=C:\Program Files\Enigma Virtual Box\enigmavbconsole.exe"
    call :print_info "检测到 Enigma Virtual Box"
)

if %HAS_UPX%==0 (
    call :print_warning "未检测到 UPX，将跳过 DLL 压缩"
    call :print_warning "下载地址: https://upx.github.io/"
)

if %HAS_ENIGMA%==0 (
    call :print_warning "未检测到 Enigma Virtual Box，将跳过单文件打包"
    call :print_warning "下载地址: https://enigmaprotector.com/en/downloads.html"
)

echo.

::===============================================================================
:: 2. 清理旧构建
::===============================================================================
call :print_info "清理旧构建..."

if exist release\ (
    rmdir /s /q release 2>nul
)
if exist dist\ (
    rmdir /s /q dist 2>nul
)
if exist Makefile (
    del /q Makefile 2>nul
)
if exist Makefile.Debug (
    del /q Makefile.Debug 2>nul
)
if exist Makefile.Release (
    del /q Makefile.Release 2>nul
)

call :print_success "清理完成"
echo.

::===============================================================================
:: 3. 编译项目
::===============================================================================
call :print_info "开始编译项目..."

qmake LockBoardTester.pro CONFIG+=release
if errorlevel 1 (
    call :print_error "qmake 配置失败"
    exit /b 1
)

%MAKE_CMD%
if errorlevel 1 (
    call :print_error "编译失败"
    exit /b 1
)

call :print_success "编译完成"
echo.

::===============================================================================
:: 4. 创建发布目录
::===============================================================================
call :print_info "创建发布目录..."

mkdir dist 2>nul
if not exist "release\LockBoardTester.exe" (
    call :print_error "未找到编译生成的 exe 文件"
    exit /b 1
)

copy /y "release\LockBoardTester.exe" "dist\" >nul
call :print_success "可执行文件已复制到 dist 目录"
echo.

::===============================================================================
:: 5. 运行 windeployqt 收集依赖
::===============================================================================
call :print_info "收集 Qt 依赖..."

cd dist
windeployqt --release --no-translations LockBoardTester.exe
if errorlevel 1 (
    call :print_error "windeployqt 执行失败"
    cd ..
    exit /b 1
)
cd ..

call :print_success "Qt 依赖收集完成"
echo.

::===============================================================================
:: 6. UPX 压缩（可选）
::===============================================================================
if %HAS_UPX%==1 (
    call :print_info "使用 UPX 压缩 DLL 文件..."
    
    set COMPRESSED_COUNT=0
    for %%f in (dist\*.dll) do (
        upx --best --lzma "%%f" >nul 2>&1
        if not errorlevel 1 (
            set /a COMPRESSED_COUNT+=1
        )
    )
    
    call :print_success "已压缩 !COMPRESSED_COUNT! 个 DLL 文件"
    echo.
)

::===============================================================================
:: 7. Enigma Virtual Box 打包（可选）
::===============================================================================
if %HAS_ENIGMA%==1 (
    call :print_info "开始单文件打包..."
    
    :: 生成 EVB 项目文件
    set EVB_PROJECT=dist\LockBoardTester.evb
    
    echo ^<?xml version="1.0" encoding="utf-8"?^> > "!EVB_PROJECT!"
    echo ^<State^> >> "!EVB_PROJECT!"
    echo   ^<InputFile^>%CD%\dist\LockBoardTester.exe^</InputFile^> >> "!EVB_PROJECT!"
    echo   ^<OutputFile^>%CD%\LockBoardTester-%VERSION%-standalone.exe^</OutputFile^> >> "!EVB_PROJECT!"
    echo   ^<Compression^>1^</Compression^> >> "!EVB_PROJECT!"
    echo   ^<Options^> >> "!EVB_PROJECT!"
    echo     ^<ShareVirtualSystem^>0^</ShareVirtualSystem^> >> "!EVB_PROJECT!"
    echo     ^<CompressFiles^>1^</CompressFiles^> >> "!EVB_PROJECT!"
    echo   ^</Options^> >> "!EVB_PROJECT!"
    echo   ^<Files^> >> "!EVB_PROJECT!"
    
    :: 添加所有 DLL 和依赖文件
    for %%f in (dist\*.dll) do (
        echo     ^<File^> >> "!EVB_PROJECT!"
        echo       ^<Type^>3^</Type^> >> "!EVB_PROJECT!"
        echo       ^<Name^>%%~nxf^</Name^> >> "!EVB_PROJECT!"
        echo       ^<File^>%%f^</File^> >> "!EVB_PROJECT!"
        echo     ^</File^> >> "!EVB_PROJECT!"
    )
    
    :: 添加 platforms 目录
    if exist "dist\platforms\" (
        for %%f in (dist\platforms\*.dll) do (
            echo     ^<File^> >> "!EVB_PROJECT!"
            echo       ^<Type^>3^</Type^> >> "!EVB_PROJECT!"
            echo       ^<Name^>platforms\%%~nxf^</Name^> >> "!EVB_PROJECT!"
            echo       ^<File^>%%f^</File^> >> "!EVB_PROJECT!"
            echo     ^</File^> >> "!EVB_PROJECT!"
        )
    )
    
    :: 添加 styles 目录
    if exist "dist\styles\" (
        for %%f in (dist\styles\*.dll) do (
            echo     ^<File^> >> "!EVB_PROJECT!"
            echo       ^<Type^>3^</Type^> >> "!EVB_PROJECT!"
            echo       ^<Name^>styles\%%~nxf^</Name^> >> "!EVB_PROJECT!"
            echo       ^<File^>%%f^</File^> >> "!EVB_PROJECT!"
            echo     ^</File^> >> "!EVB_PROJECT!"
        )
    )
    
    echo   ^</Files^> >> "!EVB_PROJECT!"
    echo ^</State^> >> "!EVB_PROJECT!"
    
    :: 执行打包
    "!ENIGMA_CLI!" "!EVB_PROJECT!"
    if errorlevel 1 (
        call :print_error "Enigma Virtual Box 打包失败"
        exit /b 1
    )
    
    if exist "LockBoardTester-%VERSION%-standalone.exe" (
        call :print_success "单文件打包完成"
        echo.
    )
)

::===============================================================================
:: 8. 输出结果
::===============================================================================
call :print_success "================================"
call :print_success "Windows 打包完成！"
call :print_success "================================"
echo.

call :print_info "输出文件："
if %HAS_ENIGMA%==1 (
    if exist "LockBoardTester-%VERSION%-standalone.exe" (
        for %%f in ("LockBoardTester-%VERSION%-standalone.exe") do (
            call :print_info "  单文件版本: %%~nxf (%%~zf 字节)"
        )
    )
)
call :print_info "  标准版本: dist\ 目录"

echo.
call :print_info "运行方式："
if %HAS_ENIGMA%==1 (
    if exist "LockBoardTester-%VERSION%-standalone.exe" (
        call :print_info "  双击 LockBoardTester-%VERSION%-standalone.exe"
    )
)
call :print_info "  或者运行 dist\LockBoardTester.exe"

echo.
call :print_success "所有操作完成！"

endlocal
exit /b 0