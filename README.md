# SunnyLand

SunnyLand是一款使用C++和SDL3开发的2D平台跳跃游戏，具有精美的像素艺术风格和流畅的游戏体验。

## 游戏特色

- 🎮 经典2D平台跳跃玩法
- 🎨 精美的像素艺术风格
- 🎵 沉浸式音效和背景音乐
- 🏆 最高分储存功能
- 📱 响应式游戏控制
- 🔄 多个游戏场景（标题、菜单、游戏、结束等）
- 💾 游戏存档功能

## 技术栈

- **编程语言**: C++23
- **游戏引擎**: 自定义引擎基于SDL3
- **构建系统**: CMake
- **依赖库**:
  - SDL3 (图形、输入、音频)
  - SDL3_image (图像加载)
  - SDL3_ttf (字体渲染)
  - SDL3_mixer (音频处理)
  - glm (数学库)
  - nlohmann_json (JSON解析)
  - spdlog (日志系统)

## 项目结构

```
SunnyLand/
├── assets/           # 游戏资源
│   ├── audio/        # 音频文件
│   ├── fonts/        # 字体文件
│   ├── maps/         # 地图文件
│   ├── textures/     # 纹理图像
│   └── config.json   # 配置文件
├── build/            # 构建输出目录
├── src/              # 源代码
│   ├── engine/       # 游戏引擎核心
│   ├── game/         # 游戏特定代码
│   └── main.cpp      # 主入口文件
├── CMakeLists.txt    # CMake构建配置
└── README.md         # 项目说明
```

## 如何构建和运行

### 前置要求

- CMake 3.16+
- C++17兼容的编译器
- Windows操作系统（目前仅支持Windows）

### 构建步骤

1. 克隆仓库
   ```bash
   git clone https://github.com/yourusername/SunnyLand.git
   cd SunnyLand
   ```

2. 配置构建
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. 编译项目
   ```bash
   cmake --build . --config Debug
   ```

4. 运行游戏
   ```bash
   # 从build目录运行
   ./SunnyLand-Windows.exe
   
   # 或从根目录运行
   ../SunnyLand-Windows.exe
   ```

## 游戏控制

- **方向键**: 移动角色
- **空格键**: 跳跃
- **Enter**: 确认/开始游戏
- **Escape**: 暂停游戏/返回菜单

## 资源说明

- **图像**: 所有游戏纹理和UI元素都位于`assets/textures/`目录
- **音频**: 音效和背景音乐位于`assets/audio/`目录
- **地图**: 游戏关卡地图位于`assets/maps/`目录
- **字体**: 游戏使用的字体位于`assets/fonts/`目录

## 开发指南

### 代码风格

- **命名约定**: 
  - 函数/变量: `camelCase`
  - 类/结构体: `PascalCase`
  - 私有成员: 前缀`m_`

- **语言标准**: C++23或更高
- **内存管理**: 优先使用RAII和智能指针
- **性能**: 大型对象使用`const reference`传递，适当使用`std::move`

### 添加新功能

1. 在`src/game/`目录下添加游戏特定代码
2. 在`src/engine/`目录下添加引擎核心功能
3. 更新`CMakeLists.txt`以包含新文件（如果需要）

## 许可证

[MIT License](LICENSE)

## 致谢

- 感谢SDL团队提供优秀的跨平台开发库
- 感谢所有贡献者和支持者

---

希望你喜欢SunnyLand！🎉
