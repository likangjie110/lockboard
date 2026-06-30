# 发布指南 / Release Guide

本文档说明如何使用 GitHub Actions 自动构建和发布 LockBoard Tester 的多平台版本。

## 📋 发布流程

### 1. 准备发布

在发布新版本之前，确保：

- [ ] 所有功能已完成并测试通过
- [ ] 代码已合并到 `main` 分支
- [ ] 更新了 `README.md` 和 `README_EN.md` 中的版本相关信息
- [ ] 更新了 `CHANGELOG.md`（如果有的话）

### 2. 创建版本标签

版本号遵循 [语义化版本规范](https://semver.org/lang/zh-CN/)：`MAJOR.MINOR.PATCH`

```bash
# 确保在 main 分支上
git checkout main
git pull origin main

# 创建标签（例如 v1.0.0）
git tag -a v1.0.0 -m "Release version 1.0.0"

# 推送标签到远程仓库（触发自动构建）
git push origin v1.0.0
```

**标签命名规则**：
- 必须以 `v` 开头，例如 `v1.0.0`
- 版本号格式：`v<major>.<minor>.<patch>`
- 示例：`v1.0.0`, `v2.1.3`, `v0.9.0-beta`

### 3. 自动构建流程

推送标签后，GitHub Actions 会自动触发构建流程：

1. **并行构建 5 个平台**：
   - Windows x64 (MinGW)
   - Windows x86 (MinGW)
   - Linux x64 (AppImage)
   - Linux ARM64 (交叉编译)
   - Linux ARMv7 (交叉编译)

2. **创建 GitHub Release**：
   - 自动生成 Release 页面
   - 上传所有平台的构建产物
   - 包含下载说明和快速开始指南

### 4. 查看构建状态

访问 GitHub Actions 页面查看构建进度：

```
https://github.com/YOUR_USERNAME/lockBoard/actions
```

或者点击仓库主页的 **Actions** 标签。

构建通常需要 **15-25 分钟**：
- Windows 构建：~5 分钟
- Linux x64 AppImage：~8 分钟
- Linux ARM 交叉编译：~10 分钟

### 5. 验证 Release

构建完成后，检查：

1. **Release 页面**：
   ```
   https://github.com/YOUR_USERNAME/lockBoard/releases
   ```

2. **确认文件**：
   - [ ] `LockBoardTester-Windows-x64.exe` (~10MB)
   - [ ] `LockBoardTester-Windows-x86.exe` (~10MB)
   - [ ] `LockBoardTester-Linux-x86_64.AppImage` (~30MB)
   - [ ] `LockBoardTester-Linux-arm64` (~5MB)
   - [ ] `LockBoardTester-Linux-armv7` (~5MB)

3. **下载测试**：
   - 下载至少一个平台的文件
   - 验证文件可以正常运行

## 🔧 版本管理

### 版本号规则

| 类型 | 说明 | 示例 |
|------|------|------|
| **Major (主版本)** | 重大功能变更、不兼容 API | `v1.0.0` → `v2.0.0` |
| **Minor (次版本)** | 新增功能、向后兼容 | `v1.0.0` → `v1.1.0` |
| **Patch (修订版)** | Bug 修复、小改进 | `v1.0.0` → `v1.0.1` |
| **预发布版** | Beta、RC 等测试版 | `v1.0.0-beta.1` |

### 推荐的发布频率

- **Patch 版本**：Bug 修复，随时发布
- **Minor 版本**：新功能完成后，每 2-4 周发布
- **Major 版本**：重大变更，根据需求发布

### 版本历史示例

```bash
v0.1.0 - 初始版本（基础功能）
v0.2.0 - 添加 ARM 支持
v0.2.1 - 修复串口连接问题
v1.0.0 - 正式版（稳定发布）
v1.1.0 - 添加日志导出功能
v1.1.1 - 修复 Windows 10 兼容性
v2.0.0 - 重构协议层（不兼容旧版本）
```

## 🚨 故障排查

### 构建失败

#### 1. Windows 构建失败

**症状**：`mingw32-make` 命令未找到

**解决方案**：
- 检查 Qt 安装配置 (`qt_arch` 参数)
- 确认使用 `win64_mingw` 或 `win32_mingw81`

#### 2. Linux AppImage 打包失败

**症状**：`linuxdeploy` 下载或执行失败

**解决方案**：
- 检查网络连接
- 工作流已包含备用方案（直接使用可执行文件）

#### 3. ARM 交叉编译失败

**症状**：编译器未找到或链接错误

**解决方案**：
- 检查交叉编译工具链安装
- 验证 `CROSS_PREFIX` 环境变量设置

#### 4. Release 创建失败

**症状**：权限错误 "Resource not accessible by integration"

**解决方案**：
```yaml
# 在 release.yml 中添加权限配置
permissions:
  contents: write
```

### 产物收集失败

#### 症状：上传产物时 "No files found"

**解决方案**：
1. 检查构建输出路径：
   ```bash
   ls -la  # Linux
   dir     # Windows
   ```

2. 调整 `output_pattern` 配置：
   ```yaml
   output_pattern: |
     **/*.exe
     **/*.AppImage
     LockBoardTester*
   ```

### 手动清理失败的 Release

如果构建失败，删除错误的 Release 和标签：

```bash
# 删除远程标签
git push --delete origin v1.0.0

# 删除本地标签
git tag -d v1.0.0

# 在 GitHub Release 页面手动删除 Release
```

## 🧪 测试发布流程

在正式发布前，建议先测试构建流程：

### 方法 1：使用测试标签

```bash
# 创建测试标签
git tag -a v0.0.1-test -m "Test release workflow"
git push origin v0.0.1-test

# 观察构建过程
# 完成后删除测试 Release 和标签
```

### 方法 2：使用 PR 测试工作流

项目包含 `.github/workflows/build-test.yml`，用于在 Pull Request 中测试编译：

```bash
# 创建功能分支
git checkout -b feature/test-build

# 提交代码并推送
git push origin feature/test-build

# 创建 Pull Request（会自动触发测试构建）
```

**build-test.yml 特点**：
- 仅验证编译通过，不创建 Release
- 在所有 PR 上自动运行
- 快速反馈代码变更的编译问题

## 📊 工作流配置说明

### release.yml 关键参数

```yaml
strategy:
  fail-fast: false  # 单个平台失败不影响其他平台
  matrix:
    config:
      - name: Windows x64
        arch: win64_mingw      # Qt 架构标识
        qt_arch: win64_mingw   # install-qt-action 参数
```

### 平台配置映射

| 目标平台 | `arch` | `qt_arch` | 说明 |
|----------|--------|-----------|------|
| Windows 64位 | `win64_mingw` | `win64_mingw` | 标准 x64 构建 |
| Windows 32位 | `win32_mingw81` | `win32_mingw81` | 32位兼容 |
| Linux x64 | `gcc_64` | `gcc_64` | 标准 Linux 构建 |
| Linux ARM | `arm64_cross` | `gcc_64` | 交叉编译（使用主机 Qt） |

### 产物命名规则

构建产物统一命名格式：

```
LockBoardTester-{Platform}-{Arch}.{ext}
```

示例：
- `LockBoardTester-Windows-x64.exe`
- `LockBoardTester-Linux-x86_64.AppImage`
- `LockBoardTester-Linux-arm64` (无扩展名)

## 📚 相关文档

- [GitHub Actions 文档](https://docs.github.com/cn/actions)
- [语义化版本规范](https://semver.org/lang/zh-CN/)
- [Qt 跨平台编译指南](https://doc.qt.io/qt-5/deployment.html)
- [项目 Linux 安装指南](linux-setup.md)

## ❓ 常见问题

### Q: 是否需要手动创建 Release？

**A**: 不需要。推送标签后，GitHub Actions 会自动创建 Release 并上传文件。

### Q: 可以修改已发布的 Release 吗？

**A**: 可以。在 GitHub Release 页面点击 "Edit release" 即可修改描述或重新上传文件。但不建议修改已发布的二进制文件（影响用户下载）。

### Q: 如何发布预览版？

**A**: 使用预发布标签：

```bash
git tag -a v1.0.0-beta.1 -m "Beta release"
git push origin v1.0.0-beta.1
```

然后在 Release 页面勾选 "This is a pre-release"。

### Q: 可以为旧版本打补丁吗？

**A**: 可以。从旧版本标签创建分支，应用修复后创建新标签：

```bash
git checkout -b hotfix/v1.0 v1.0.0
# 应用修复...
git commit -m "Fix critical bug"
git tag -a v1.0.1 -m "Hotfix release"
git push origin v1.0.1
```

## 🎯 首次发布检查清单

在首次运行自动发布流程前，确认：

- [ ] `.github/workflows/release.yml` 已创建
- [ ] 仓库 Settings → Actions → General 中启用了 Actions
- [ ] 仓库 Settings → Actions → Workflow permissions 设置为 "Read and write permissions"
- [ ] README 中添加了 Release 徽章
- [ ] 创建了测试标签验证流程
- [ ] 文档已更新（本文档和 README）

---

**维护者**：请保持本文档与实际工作流配置同步。

**最后更新**：2024-06-30