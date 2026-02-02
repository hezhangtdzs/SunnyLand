# SunnyLand

一个基于 **C++23** 开发的 2D 平台跳跃游戏，采用现代化的组件化架构设计。

![Preview](assets/textures/Preview.png)

## 项目简介

SunnyLand 是一款横版平台跳跃游戏，玩家控制角色在精美的像素风关卡中冒险，收集道具、击败敌人、到达终点。项目采用现代化的游戏引擎架构，展示了多种经典设计模式在实战中的应用。

## 核心亮点

### 架构设计

| 设计模式 | 应用场景 | 优势 |
|:---|:---|:---|
| **组件化架构** | 游戏对象系统 | 灵活组合，避免深层继承 |
| **生成器模式** | 复杂对象构建 | 分步构建，链式配置 |
| **命令模式** | 输入处理解耦 | 支持控制切换，易于扩展 |
| **状态模式** | 玩家行为管理 | 清晰的状态转换逻辑 |
| **观察者模式** | 数据驱动 UI | 自动响应数据变化 |
| **服务定位器** | 音频系统访问 | 全局访问，解耦依赖 |
| **装饰器模式** | 音频功能扩展 | 运行时动态组合功能 |

### 技术特色

- **C++23 现代特性** - 使用结构化绑定、概念、constexpr if 等现代语法
- **数据驱动设计** - JSON 配置 + Tiled 地图编辑器，无需修改代码即可调整游戏
- **智能内存管理** - 全程使用 `std::unique_ptr`/`std::shared_ptr`，杜绝内存泄漏
- **跨平台支持** - Windows/Linux/macOS 全平台兼容

## 技术栈

| 技术 | 用途 |
|:---|:---|
| **C++23** | 主开发语言 |
| **CMake 3.10+** | 构建系统 |
| **SDL3** | 窗口、输入、渲染 |
| **SDL3_image/mixer/ttf** | 图像、音频、字体 |
| **GLM** | 数学库 |
| **nlohmann/json** | JSON 解析 |
| **spdlog** | 日志系统 |

## 快速开始

### 环境要求

- Windows 10/11 / Linux / macOS
- Visual Studio 2022 / GCC 11+ / Clang 14+
- CMake 3.10+
- vcpkg

### 一键构建

```bash
# 安装依赖 (vcpkg)
vcpkg install sdl3 sdl3-image sdl3-mixer sdl3-ttf glm nlohmann-json spdlog

# 克隆并构建
git clone <repository-url>
cd SunnyLand
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build .

# 运行
./SunnyLand-Windows.exe
```

详细构建指南请参阅 [docs/architecture/10_build_guide.md](docs/architecture/10_build_guide.md)

## 游戏操作

| 操作 | 按键 | 说明 |
|:---|:---|:---|
| 移动 | `A` `D` / `←` `→` | 左右移动 |
| 跳跃 | `Space` / `J` | 跳跃（支持土狼时间） |
| 攻击 | `K` / 鼠标左键 | 攻击敌人 |
| 攀爬 | `W` `S` / `↑` `↓` | 在梯子上攀爬 |
| 暂停 | `P` / `Esc` | 打开暂停菜单 |

## 项目结构

```
SunnyLand/
├── assets/              # 游戏资源（音频、纹理、地图、配置）
├── docs/                # 架构文档
│   └── architecture/    # 详细设计文档
├── src/
│   ├── engine/          # 游戏引擎核心
│   │   ├── audio/       # 音频系统（服务定位器 + 装饰器模式）
│   │   ├── component/   # 组件系统
│   │   ├── core/        # 核心模块（Context、GameApp）
│   │   ├── physics/     # 物理引擎（碰撞检测、斜坡物理）
│   │   ├── render/      # 渲染系统
│   │   ├── scene/       # 场景管理（栈式管理）
│   │   └── ui/          # UI 系统（状态机驱动）
│   └── game/            # 游戏业务逻辑
│       ├── component/   # 玩家组件、AI 组件
│       ├── data/        # 会话数据（观察者模式）
│       └── scene/       # 游戏场景实现
└── CMakeLists.txt
```

## 架构文档

| 文档 | 内容概述 |
|:---|:---|
| [01_project_overview.md](docs/architecture/01_project_overview.md) | 项目概述与 9 种设计模式详解 |
| [02_core_class_diagram.md](docs/architecture/02_core_class_diagram.md) | 完整类图与关系说明 |
| [03_main_loop_sequence.md](docs/architecture/03_main_loop_sequence.md) | 游戏主循环时序图 |
| [04_directory_structure.md](docs/architecture/04_directory_structure.md) | 目录结构详细说明 |
| [05-08_key_mechanisms_part1-4.md](docs/architecture/05_key_mechanisms_part1.md) | 核心机制详解（4 部分） |
| [09_development_guidelines.md](docs/architecture/09_development_guidelines.md) | 开发规范与最佳实践 |
| [10_build_guide.md](docs/architecture/10_build_guide.md) | 完整构建指南 |
| [11_api_reference.md](docs/architecture/11_api_reference.md) | 核心 API 快速参考 |

## 核心代码示例

### 组件系统使用

```cpp
// 创建玩家对象
auto player = std::make_unique<GameObject>("player", "hero");
player->addComponent<TransformComponent>(glm::vec2(100, 200));
player->addComponent<SpriteComponent>("player_texture");
player->addComponent<PhysicsComponent>(&physics_engine);
player->addComponent<PlayerComponent>();  // 状态机驱动
scene->safeAddGameObject(std::move(player));
```

### 命令模式实现输入

```cpp
// 绑定命令
command_mapper_->bind("jump", 
    std::make_unique<JumpCommand>(player_component_));

// 输入处理
if (input.isActionPressed("jump")) {
    command_mapper_->execute("jump", context_);
}
```

### 观察者模式实现数据驱动 UI

```cpp
// SessionData 通知数据变化
void addScore(int score) {
    current_score_ += score;
    notifyObservers(EventType::SCORE_CHANGED, current_score_);
}

// UI 自动更新
void onNotify(EventType type, const std::any& data) override {
    if (type == EventType::SCORE_CHANGED) {
        setText("Score: " + std::to_string(std::any_cast<int>(data)));
    }
}
```

## 游戏特性

- **流畅的物理系统** - 支持重力、斜坡、梯子、碰撞检测
- **智能敌人 AI** - 巡逻、跳跃、上下飞行多种行为模式
- **完整的 UI 系统** - 主菜单、暂停菜单、HUD、结束画面
- **存档系统** - 自动保存进度，支持手动存档
- **计分系统** - 收集物品、击败敌人获得分数
- **多关卡支持** - 数据驱动关卡切换

## 开发规范

- **命名**: 类名 `PascalCase`，函数/变量 `camelCase`，私有成员 `m_` 前缀
- **内存**: 全程智能指针，禁止裸 `new/delete`
- **文档**: Doxygen 风格注释
- **标准**: C++23 现代特性优先

## 关卡设计

使用 [Tiled Map Editor](https://www.mapeditor.org/) 创建关卡：

1. 创建地图，设置图块大小（18x18）
2. 导入图块集，设计关卡布局
3. 添加对象层放置敌人、道具
4. 导出 JSON 到 `assets/maps/`

## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 致谢

- [SDL](https://www.libsdl.org/) · [GLM](https://glm.g-truc.net/) · [nlohmann/json](https://github.com/nlohmann/json) · [spdlog](https://github.com/gabime/spdlog) · [Tiled](https://www.mapeditor.org/)

---

**注意**: 本项目为学习用途开发，资源文件仅供演示使用。
