# 关键机制说明 (Key Mechanisms) - 第三部分

## 11. 碰撞系统 (Collision)

- **Collider / ColliderType**：形状定义，当前支持 `AABB` 与 `CIRCLE`。
- **ColliderComponent**：挂载到 `GameObject` 上，持有 `Collider`，并可计算世界坐标 AABB（`getWorldAABB()`）。
- **collision 命名空间**：提供检测函数（`checkCollision`、`checkAABBOverlap`、`checkCircleOverlap`、`checkPointInCircle` 等）。
- **接入点**：`PhysicsEngine::checkObjectCollisions()` 调用 `collision::checkCollision()`，并记录碰撞对到 `collision_pairs_`。

## 12. 数学工具 (Math Utilities)

本项目提供了一些数学工具类，用于游戏开发中的常见数学运算。

### FColor 结构体
- **定义位置**: `engine::utils::FColor` 定义在 `math.h` 文件中。
- **功能**: 提供浮点数表示的颜色结构体，使用 0.0f-1.0f 范围的 RGBA 值。
- **优势**: 相比 `SDL_Color` (0-255 范围的整数)，`FColor` 提供了更精确的颜色表示，便于进行颜色插值和计算。
- **使用场景**: 广泛应用于 UI 系统、渲染系统和特效系统中，确保颜色表示的一致性。

## 13. Transform / Alignment 约定（渲染与碰撞坐标基准）

本项目中多个组件都会把 `TransformComponent::position` 当作“锚点”，其实际世界左上角由各组件的 `Alignment` 计算出的 `offset_` 决定：

- `SpriteComponent::render()` 使用 `draw_pos = transform_pos + sprite_offset_`。
- `ColliderComponent::getWorldAABB()` 使用 `aabb_top_left = transform_pos + collider_offset_`。

注意：

- `SpriteComponent` 的默认 `alignment` 为 `Alignment::NONE`，在现实现中等价于不做偏移（`offset_ = (0,0)`），也就是把 `Transform.position` 当作贴图左上角。
- `ColliderComponent` 若设置为 `Alignment::CENTER`，会产生 `offset_ = (-w/2, -h/2)`，此时 `Transform.position` 被当作碰撞盒中心。

因此如果只把 Collider 改为 `CENTER` 而 Sprite 仍为默认 `NONE`，会出现“碰撞盒贴地但贴图嵌入地面/错位”的视觉现象。

建议：

- 让 `SpriteComponent` 与 `ColliderComponent` 使用一致的 `Alignment`（例如都用 `TOP_LEFT` 或都用 `CENTER`）。
- 或制定全局约定：`Transform.position` 永远表示左上角/永远表示中心，并让所有组件遵循。

## 14. 玩家状态机 (Player State Machine)

本项目使用 **状态模式 (State Pattern)** 管理玩家的复杂行为逻辑。

- **PlayerComponent**: 玩家的核心组件，持有当前状态 (`current_state_`) 和物理属性（速度参数、跳跃力等）。
  - 提供 `setState()` 切换状态（自动调用 `exit()` 和 `enter()`）。
  - 提供 `processMovementInput()` 辅助函数，供各状态调用以处理通用的左右移动逻辑（施加力、翻转精灵、处理转向瞬间的速度归零）。

- **PlayerState**: 所有玩家状态的基类 (抽象类)。
  - `enter()`: 进入状态时调用（初始化动画、重置变量）。
  - `exit()`: 退出状态时调用（清理）。
  - `handleInput()`: 处理输入，返回新状态（`unique_ptr`）或 `nullptr`（保持当前状态）。
  - `update()`: 帧更新逻辑，处理物理检测、自动状态切换（如落地、起跳）。

- **具体状态**:
  - `IdleState`: 待机状态。检测移动输入切换 `WalkState`，检测跳跃切换 `JumpState`，检测下落。
  - `WalkState`: 移动状态。处理移动输入，检测停止输入回 `IdleState`。
  - `JumpState`: 跳跃上升状态。进入时施加向上速度，并进行微小的垂直位移（抬升）以确保离开地表。空中可移动，检测 Y 轴速度向下切换 `FallState`。
  - `FallState`: 下落状态。空中可移动。检测地面碰撞 (`hasCollidedBelow`) 切换 `Idle/Walk`。
  - `HurtState`: 受伤状态。进入时播放受伤动画，通常在受击瞬间从其他状态切入。
  - `DeadState`: 死亡状态。进入时播放死亡动画，禁用大部分输入处理，准备从场景移除。
  - `ClimbState`: 攀爬状态。在梯子范围内触发。进入时禁用重力，允许垂直移动和微量水平位移，通过检测梯子范围自动退出或切出跳转。

## 15. 生命值系统 (Health System)

- **HealthComponent**: 独立管理实体的生命值生命周期。
  - **无敌帧 (Invincibility Frames)**: 采用计时器机制。受伤后 `invincibility_timer_` 被设为 `invincibility_duration_`，随后随时间递减（在 `update` 中处理）。只要计时器大于 0，`isInvincible()` 即返回 true，实体免受伤害。
  - **受击逻辑**: `takeDamage()` 包含无敌状态检查。扣血成功后，会自动触发无敌计时。
- **与状态机连接**: `PlayerComponent` 监听生命值变化并根据无敌计时器驱动 `SpriteComponent` 实现闪烁反馈。当 `HealthComponent` 触发扣血时，`PlayerComponent` 会切换到 `HurtState` 或 `DeadState`。

## 16. 危险处理与场景反馈 (Hazard Processing & Feedback)

- **处理统一化**: `GameScene::processHazardDamage()` 统一处理玩家受到的环境伤害（如尖刺瓦片或尖刺对象），确保伤害判定的触发逻辑一致。
- **效果对齐**: `GameScene::createEffect()` 支持多种标签（如 "enemy", "item"），并根据动画帧尺寸自动应用位置偏移，确保特效中心点与触发位置精准重叠。

## 17. 状态机与物理系统的交互 (Physics & State Machine Interaction)

- **碰撞标记 (Collision Flags)**: `PhysicsComponent` 实时维护四个方向的碰撞标记：`below`, `above`, `left`, `right`。
- **状态切换依据**: 
  - `IdleState` 和 `WalkState` 会每帧检查 `!hasCollidedBelow()`，如果为真则代表玩家悬空，此时会设置“土狼计时器”并立即切换到 `FallState`。
  - `FallState` 会检查 `coyote_timer_`，如果大于 0 且按下跳跃键，则允许返回 `JumpState`。
  - `FallState` 会每帧检查 `hasCollidedBelow()`，如果为真则代表玩家落地，立即切换到 `IdleState` 或 `WalkState`。
  - **攀爬交互**: `ClimbState` 会修改 `PhysicsComponent` 的 `climbing` 标志并调用 `setUseGravity(false)`。在更新逻辑中通过在头、中、足三个关键采样点探测 `LADDER` 瓦片来判断玩家是否仍处于梯子有效区域。
- **输入处理辅助**: `PlayerComponent::processMovementInput()` 封装了左右移动的力学逻辑。在空中状态（`Jump/Fall`）调用时通常会传入较小的速度系数（如 `0.5f`）以实现受限的空中控制。

## 18. AI 行为系统 (AI Behavior System)

本项目采用 **策略模式 (Strategy Pattern)** 实现敌人的 AI 逻辑。

- **AIComponent**: 挂载到敌对 NPC 上的组件，内部持有一个 `AIBehavior` 策略对象。
  - 每帧驱动 `current_behavior_->update()`，将具体逻辑逻辑委托给策略类。
- **AIBehavior (抽象基类)**: 定义了统一的 `update(delta_time, game_object)` 接口。
- **具体策略**:
  - `PatrolBehavior`: 基础左右巡逻。支持设置巡逻范围和移动速度。
  - `UpDownBehavior`: 垂直飞行巡逻。常用于 Eagle 等飞行敌人。
  - `JumpBehavior`: 周期性跳跃移动。模拟 Frog 的动作，结合物理引擎实现抛物线跳跃。

## 19. 战斗判定 (Combat Detection)

- **踩踏判定 (Stomp Logic)**:
  - 优化后的 `PlayerVSEnemyCollision` 不再仅依赖重叠区域的长宽比。
  - **判定条件**: 1. 玩家垂直速度向下 (`velocity.y > 0`)； 2. 玩家足部 AABB 边界在敌人中心线上方。
  - **效果**: 判定成功则调用敌人 `HealthComponent` 扣血，并赋予玩家瞬时的向上冲量（反弹跳）。判定失败则玩家受损。