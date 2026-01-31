# SunnyLand

一个基于 C++23 开发的 2D 平台跳跃游戏，采用现代化的组件化架构设计。

![Preview](assets/textures/Preview.png)

## 项目概述

SunnyLand 是一个横版平台跳跃游戏，玩家控制角色在多个关卡中冒险，收集道具、击败敌人、到达终点。项目采用组件化架构，支持数据驱动的关卡设计和灵活的游戏对象构建。

### 核心特性

- **组件化架构** - 通过组件组合实现游戏对象功能，灵活且易于扩展
- **数据驱动** - 使用 JSON 配置文件和 Tiled 地图编辑器进行关卡设计
- **生成器模式** - 使用 Builder 模式构建复杂的游戏对象
- **状态模式** - 管理玩家行为和 UI 交互状态
- **策略模式** - 实现可替换的 AI 行为
- **资源共享** - 智能指针管理资源生命周期，避免重复加载

## 技术栈

| 技术 | 版本 | 用途 |
|:---|:---|:---|
| **C++** | C++23 | 主开发语言 |
| **CMake** | 3.10+ | 构建系统 |
| **SDL3** | 最新版 | 窗口管理、输入、渲染 |
| **GLM** | - | 数学库（向量、矩阵） |
| **nlohmann/json** | - | JSON 解析 |
| **spdlog** | - | 日志系统 |
| **Tiled** | - | 地图编辑器 |
| **SDL3_mixer** | - | 音频解码与混音 |
| **SDL3_ttf** | - | 字体渲染 |

## 快速开始

### 环境要求

- Windows 10/11
- Visual Studio 2022 或更高版本
- CMake 3.10 或更高版本
- vcpkg（用于管理依赖库）

### 安装依赖

使用 vcpkg 安装所需的库：

```bash
vcpkg install sdl3 sdl3-image sdl3-mixer sdl3-ttf glm nlohmann-json spdlog
```

### 构建项目

```bash
# 克隆项目
git clone <repository-url>
cd SunnyLand

# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake

# 构建
cmake --build .
```

### 运行游戏

构建完成后，运行生成的可执行文件：

```bash
./SunnyLand-Windows.exe
```

## 游戏操作

| 操作 | 键盘 | 说明 |
|:---|:---|:---|
| 移动 | A / D 或 ← / → | 左右移动 |
| 跳跃 | Space / J | 跳跃 |
| 下蹲 | S 或 ↓ | 下蹲/下滑 |
| 攻击 | K 或 鼠标左键 | 攻击 |

## 项目结构

```
SunnyLand/
├── assets/                 # 游戏资源
│   ├── audio/             # 音频文件
│   ├── fonts/             # 字体文件
│   ├── maps/              # Tiled 地图文件
│   ├── textures/          # 图片资源
│   └── config.json        # 游戏配置文件
├── docs/                  # 项目文档
│   └── architecture/      # 架构文档
├── src/                   # 源代码
│   ├── engine/            # 游戏引擎
│   │   ├── audio/         # 音频系统
│   │   ├── component/     # 组件系统
│   │   ├── core/          # 核心模块
│   │   ├── input/         # 输入管理
│   │   ├── object/        # 游戏对象
│   │   ├── physics/       # 物理引擎
│   │   ├── render/        # 渲染系统
│   │   ├── resource/      # 资源管理
│   │   ├── scene/         # 场景管理
│   │   └── ui/            # UI 系统
│   └── game/              # 游戏逻辑
│       ├── component/     # 游戏组件
│       ├── data/          # 数据管理
│       ├── object/        # 游戏对象构建器
│       └── scene/         # 游戏场景
├── build/                 # 构建输出
├── CMakeLists.txt         # CMake 配置
└── README.md              # 项目说明
```

## 架构文档

详细的架构文档位于 `docs/architecture/` 目录：

| 文档 | 内容 |
|:---|:---|
| [01_project_overview.md](docs/architecture/01_project_overview.md) | 项目概述和设计模式 |
| [02_core_class_diagram.md](docs/architecture/02_core_class_diagram.md) | 核心类图和关系 |
| [03_main_loop_sequence.md](docs/architecture/03_main_loop_sequence.md) | 主循环时序图 |
| [04_directory_structure.md](docs/architecture/04_directory_structure.md) | 目录结构说明 |
| [05_key_mechanisms_part1.md](docs/architecture/05_key_mechanisms_part1.md) | 核心机制（一）对象构建 |
| [06_key_mechanisms_part2.md](docs/architecture/06_key_mechanisms_part2.md) | 核心机制（二）组件系统 |
| [07_key_mechanisms_part3.md](docs/architecture/07_key_mechanisms_part3.md) | 核心机制（三）物理与碰撞 |
| [08_key_mechanisms_part4.md](docs/architecture/08_key_mechanisms_part4.md) | 核心机制（四）场景与数据 |
| [09_development_guidelines.md](docs/architecture/09_development_guidelines.md) | 开发规范与指南 |

## 核心机制

### 1. 组件系统

游戏对象通过挂载组件实现功能：

```cpp
auto player = std::make_unique<GameObject>();
player->addComponent<TransformComponent>(position);
player->addComponent<SpriteComponent>(texture);
player->addComponent<PhysicsComponent>();
player->addComponent<PlayerComponent>();
```

### 2. 生成器模式

使用 Builder 模式构建复杂游戏对象：

```cpp
GameObjectBuilder builder(level_loader, context);
auto enemy = builder
    .configure(&object_json, &tile_json, tile_info)
    .setEnemyType("eagle")
    .build()
    .getGameObject();
```

### 3. 状态管理

玩家状态使用状态模式管理：

```cpp
// 状态转换示例
void WalkState::handleInput(PlayerComponent& player, const InputManager& input) {
    if (!input.isKeyPressed(SDL_SCANCODE_A) && !input.isKeyPressed(SDL_SCANCODE_D)) {
        player.changeState(std::make_unique<IdleState>());
    }
}
```

## 配置说明

游戏配置存储在 `assets/config.json` 中：

```json
{
    "audio": {
        "master_volume": 0.5,
        "music_volume": 0.5,
        "sound_volume": 0.5
    },
    "graphics": {
        "vsync": true
    },
    "input_mappings": {
        "jump": ["J", "Space"],
        "attack": ["K", "MouseLeft"]
    }
}
```

## 开发规范

### 命名规范

- **类名**: `PascalCase` (如 `GameObject`, `PhysicsComponent`)
- **函数名**: `camelCase` (如 `update()`, `render()`)
- **变量名**: `camelCase` (如 `position`, `velocity`)
- **私有成员**: 前缀 `m_` (如 `m_position`, `m_velocity`)
- **宏/常量**: `UPPER_SNAKE_CASE`

### 代码风格

- 使用 C++23 现代特性（auto、智能指针、结构化绑定）
- 优先使用 RAII 和智能指针管理内存
- 大型对象使用 `const reference` 传递
- 使用 `std::move` 优化性能

### 文档注释

使用 Doxygen 风格注释：

```cpp
/**
 * @brief 更新游戏对象状态
 * @param dt 帧间隔时间（秒）
 */
void update(float dt);
```

## 关卡设计

使用 [Tiled Map Editor](https://www.mapeditor.org/) 创建关卡：

1. 创建新地图，设置图块大小（推荐 18x18）
2. 导入图块集（`assets/textures/Layers/tileset.png`）
3. 设计关卡地图
4. 添加对象层放置敌人、道具等
5. 导出为 JSON 格式到 `assets/maps/`

## 贡献指南

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 致谢

- [SDL](https://www.libsdl.org/) - 跨平台开发库
- [GLM](https://glm.g-truc.net/) - 数学库
- [nlohmann/json](https://github.com/nlohmann/json) - JSON 库
- [spdlog](https://github.com/gabime/spdlog) - 日志库
- [Tiled](https://www.mapeditor.org/) - 地图编辑器

---

**注意**: 本项目为学习用途开发，资源文件仅供演示使用。
