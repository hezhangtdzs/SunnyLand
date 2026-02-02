# 构建指南 (Build Guide)

本指南介绍如何构建 SunnyLand 项目。

## 系统要求

### 最低配置

| 组件 | 要求 |
|:---|:---|
| 操作系统 | Windows 10/11, Linux, macOS |
| 编译器 | MSVC 2019+ (Windows), GCC 11+ (Linux), Clang 14+ (macOS) |
| CMake | 3.10 或更高版本 |
| C++ 标准 | C++23 |

### 依赖库

项目使用以下第三方库，通过 vcpkg 管理：

| 库名 | 用途 | vcpkg 包名 |
|:---|:---|:---|
| SDL3 | 窗口、输入、渲染 | `sdl3` |
| SDL3_image | 图像加载 | `sdl3-image` |
| SDL3_mixer | 音频播放 | `sdl3-mixer` |
| SDL3_ttf | 文本渲染 | `sdl3-ttf` |
| GLM | 数学库 | `glm` |
| nlohmann-json | JSON 解析 | `nlohmann-json` |
| spdlog | 日志系统 | `spdlog` |

## 环境配置

### Windows (使用 vcpkg)

#### 1. 安装 vcpkg

```powershell
# 克隆 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# 安装 vcpkg
.\bootstrap-vcpkg.bat

# 添加到环境变量（可选）
# 将 vcpkg 目录添加到 PATH
```

#### 2. 安装依赖库

```powershell
# 安装所有依赖（x64-windows 平台）
.\vcpkg install sdl3:x64-windows sdl3-image:x64-windows sdl3-mixer:x64-windows sdl3-ttf:x64-windows glm:x64-windows nlohmann-json:x64-windows spdlog:x64-windows

# 或者使用集成模式
.\vcpkg integrate install
```

#### 3. 配置 CMake

```powershell
# 创建构建目录
mkdir build
cd build

# 配置（使用 vcpkg 工具链）
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake

# 或者如果已集成 vcpkg
cmake ..
```

### Linux (Ubuntu/Debian)

#### 1. 安装系统依赖

```bash
# 安装编译工具和 CMake
sudo apt update
sudo apt install -y build-essential cmake git

# 安装 SDL3 开发库
sudo apt install -y libsdl3-dev libsdl3-image-dev libsdl3-mixer-dev libsdl3-ttf-dev

# 安装其他依赖
sudo apt install -y libglm-dev nlohmann-json3-dev libspdlog-dev
```

#### 2. 构建项目

```bash
# 创建构建目录
mkdir build
cd build

# 配置
cmake ..

# 编译
make -j$(nproc)
```

### macOS

#### 1. 安装 Homebrew 依赖

```bash
# 安装 CMake
brew install cmake

# 安装 SDL3
brew install sdl3 sdl3_image sdl3_mixer sdl3_ttf

# 安装其他依赖
brew install glm nlohmann-json spdlog
```

#### 2. 构建项目

```bash
# 创建构建目录
mkdir build
cd build

# 配置
cmake ..

# 编译
make -j$(sysctl -n hw.ncpu)
```

## CMake 配置选项

### 编译选项

| 选项 | 默认值 | 说明 |
|:---|:---|:---|
| `ENABLE_AUDIO_LOG` | `OFF` | 启用音频日志装饰器 |
| `CMAKE_BUILD_TYPE` | `Release` | 构建类型 (Debug/Release) |

### 使用示例

```bash
# Debug 构建，启用音频日志
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_AUDIO_LOG=ON

# Release 构建
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## 构建步骤

### 完整构建流程

```bash
# 1. 克隆项目
git clone <项目仓库>
cd SunnyLand

# 2. 创建构建目录
mkdir build
cd build

# 3. 配置项目（Windows 需要指定 vcpkg 工具链）
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake

# 4. 编译
cmake --build . --config Release

# 5. 运行
./SunnyLand-Windows.exe  # Windows
./SunnyLand-Linux        # Linux
./SunnyLand-Darwin       # macOS
```

### Visual Studio 构建

```powershell
# 生成 Visual Studio 项目
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake

# 打开生成的 .sln 文件，或使用命令行构建
cmake --build . --config Release
```

## 项目结构

构建完成后，输出目录结构：

```
SunnyLand/
├── SunnyLand-Windows.exe  # 可执行文件
├── assets/                # 资源目录
│   ├── audio/            # 音频文件
│   ├── textures/         # 纹理文件
│   ├── maps/             # 关卡地图
│   ├── fonts/            # 字体文件
│   └── config.json       # 配置文件
└── docs/                 # 文档
```

## 故障排除

### 常见问题

#### 1. CMake 找不到 SDL3

**问题**: `Could not find a package configuration file provided by "SDL3"`

**解决**:
- 确保 vcpkg 已正确安装 SDL3
- 检查 `CMAKE_TOOLCHAIN_FILE` 路径是否正确
- 尝试重新安装: `vcpkg install sdl3 --recurse`

#### 2. 编译错误：C++23 特性不支持

**问题**: 编译器报错关于 C++23 特性

**解决**:
- 升级编译器到支持 C++23 的版本
- Windows: Visual Studio 2022 17.4+
- Linux: GCC 11+ 或 Clang 14+
- macOS: Xcode 14+

#### 3. 运行时找不到 DLL

**问题**: 提示缺少 `SDL3.dll` 等动态库

**解决**:
- 将 vcpkg 的 `installed/x64-windows/bin` 目录中的 DLL 复制到可执行文件目录
- 或添加 vcpkg bin 目录到系统 PATH

#### 4. 音频日志宏未生效

**问题**: 设置了 `ENABLE_AUDIO_LOG=ON` 但无日志输出

**解决**:
- 确保重新运行 CMake 配置
- 检查 spdlog 日志级别是否设置为 `info` 或更低

### 调试构建

```bash
# Debug 模式构建
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 使用调试器运行
gdb ./SunnyLand-Linux
lldb ./SunnyLand-Darwin
```

## 高级配置

### 静态链接

```bash
# 静态链接所有依赖（Windows）
cmake .. -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

### 自定义安装路径

```bash
# 指定安装路径
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install

# 安装
cmake --build . --target install
```

## 持续集成

### GitHub Actions 示例

```yaml
name: Build

on: [push, pull_request]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11
        with:
          vcpkgGitCommitId: '...'
      
      - name: Configure CMake
        run: cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${{ env.VCPKG_ROOT }}/scripts/buildsystems/vcpkg.cmake
      
      - name: Build
        run: cmake --build build --config Release
```

## 参考资源

- [CMake 官方文档](https://cmake.org/documentation/)
- [vcpkg 官方文档](https://vcpkg.io/en/docs/)
- [SDL3 官方文档](https://wiki