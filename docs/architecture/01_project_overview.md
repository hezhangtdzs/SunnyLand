# 项目概述 (Project Overview)

本项目是一个基于 **C++23** 开发的 2D 平台跳跃游戏，采用现代化的 **组件化架构 (Component-Based Architecture)**。游戏引擎设计遵循以下核心原则：

- **组合优于继承**: 通过组件组合实现游戏对象的功能
- **数据驱动**: 使用 JSON 配置文件和 Tiled 地图编辑器进行关卡设计
- **资源共享**: 通过智能指针管理资源生命周期，避免重复加载
- **安全的内存管理**: 延迟对象添加/删除，避免迭代器失效
- **生成器模式**: 使用 Builder 模式构建复杂的游戏对象，支持灵活的对象配置

## 架构设计模式

### 1. 组件化架构 (Component-Based Architecture)
游戏对象 (`GameObject`) 本身不包含具体功能，而是通过挂载各种组件 (`Component`) 来实现行为。这种设计允许：
- 灵活组合对象行为
- 运行时动态添加/移除功能
- 避免深层继承层次

### 2. 生成器模式 (Builder Pattern)
使用生成器模式构建复杂的游戏对象，主要包含：
- **ObjectBuilder**: 抽象生成器，定义构建游戏对象的通用步骤
- **GameObjectBuilder**: 具体生成器，实现游戏特定对象的构建逻辑

优势：
- 将复杂对象的构建过程分解为步骤
- 支持链式调用配置对象
- 允许通过增强模式为已有对象添加组件

### 3. 状态模式 (State Pattern)
用于管理玩家行为和 UI 交互状态：
- **PlayerState**: 管理玩家各种状态（Idle、Walk、Jump、Fall 等）
- **UIState**: 管理 UI 元素交互状态（Normal、Hover、Pressed）

### 4. 命令模式 (Command Pattern)
用于解耦输入处理与动作执行，实现玩家动作的灵活控制：
- **PlayerComponent**: 接收输入并调用动作接口
- **PlayerState**: 各状态类实现具体的动作响应逻辑
- **动作接口**: `moveLeft()`, `moveRight()`, `jump()`, `attack()`, `climbUp()`, `climbDown()`, `stopMove()`

优势：
- 输入处理与动作执行解耦
- 状态类自主决定如何处理特定动作
- 易于扩展新动作和状态

### 5. 策略模式 (Strategy Pattern)
用于实现可替换的 AI 行为：
- **AIBehavior**: 定义 AI 行为接口
- **PatrolBehavior / UpDownBehavior / JumpBehavior**: 具体 AI 策略实现

### 6. 装饰器模式 (Decorator Pattern)
用于动态扩展音频系统功能，不修改原有代码：
- **LogAudioPlayer**: 装饰器，为 AudioPlayer 添加日志记录功能
- 通过编译宏 `ENABLE_AUDIO_LOG` 动态启用/禁用
- 支持嵌套装饰，可扩展更多功能（如统计、缓存等）

优势：
- 遵循开闭原则，无需修改 AudioPlayer 代码
- 职责分离，日志记录与音频播放逻辑解耦
- 运行时动态组合功能

### 7. 空对象模式 (Null Object Pattern)
用于提供服务定位器的默认实现，避免空指针检查：
- **NullAudioPlayer**: 实现 IAudioPlayer 接口但无实际操作
- 在服务未注册时提供默认行为
- 客户端无需检查服务是否存在

优势：
- 消除空指针检查代码
- 保持代码简洁和一致性
- 与服务定位器模式完美配合

### 8. 服务定位器模式 (Service Locator Pattern)
作为依赖注入的补充方案，提供全局访问音频系统的能力：
- **AudioLocator**: 提供全局访问点 `AudioLocator::get()`
- **IAudioPlayer**: 音频服务接口
- **AudioPlayer**: 具体音频实现

使用场景：
- 音频播放可能发生在游戏任何角落（UI、组件、场景）
- 减少构造函数参数传递的复杂性
- 与依赖注入形成互补

### 9. 脏标识模式 (Dirty Flag Pattern)
用于优化 UI 文本渲染性能，避免每帧执行昂贵操作：
- **UIText**: UI层维护 `is_dirty_` 标记
- **TextRenderer**: 渲染层根据脏标记决定是否同步状态
- 将昂贵的文本测量和渲染状态同步推迟到必要时执行

优势：
- 显著减少每帧的 GPU 和 CPU 开销
- 延迟计算，避免不必要的中间操作
- 自动管理，使用简单透明

## 技术栈 (Tech Stack)

| 技术 | 版本 | 用途 |
|:---|:---|:---|
| **C++** | C++23 | 主开发语言，使用现代特性如结构化绑定、概念等 |
| **CMake** | 3.10+ | 构建系统 |
| **SDL3** | 最新版 | 窗口管理、输入、渲染 |
| **SDL3_image** | 最新版 | 图像加载（PNG、JPG 等格式支持） |
| **GLM** | - | 数学库（向量、矩阵） |
| **nlohmann/json** | - | JSON 解析 |
| **spdlog** | - | 日志系统 |
| **Tiled** | - | 地图编辑器 |
| **SDL3_mixer** | - | 音频解码与混音（BGM/SFX） |
| **SDL3_ttf** | - | 文本渲染（字体加载与文本绘制） |
| **Physics (Custom)** | - | 轻量 2D 物理（重力/速度/受力积分） |