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

## 技术栈 (Tech Stack)

| 技术 | 版本 | 用途 |
|:---|:---|:---|
| **C++** | C++23 | 主开发语言 |
| **CMake** | 3.0+ | 构建系统 |
| **SDL3** | 最新版 | 窗口管理、输入、渲染 |
| **GLM** | - | 数学库（向量、矩阵） |
| **nlohmann/json** | - | JSON 解析 |
| **spdlog** | - | 日志系统 |
| **Tiled** | - | 地图编辑器 |
| **SDL3_mixer** | - | 音频解码与混音（BGM/SFX） |
| **SDL3_ttf** | - | 文本渲染（字体加载与文本绘制） |
| **Physics (Custom)** | - | 轻量 2D 物理（重力/速度/受力积分） |