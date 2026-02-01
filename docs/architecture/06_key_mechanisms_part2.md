# 关键机制说明 (Key Mechanisms) - 第二部分

## 7. 关卡加载 (Level Loading)
- **从 Tiled 导入**: 使用 `LevelLoader` 解析 Tiled 编辑器导出的 JSON 格式 (`.tmj`) 地图。
- **图块集支持**: 支持解析外部Tileset (`.tsj`)，包括大图 (`Image Collection`) 和单图集 (`Image`) 模式。
- **图层解析**: 
    - `Image Layer` -> `ParallaxComponent` (支持部分滚动因子，用于远景)。
    - `Tile Layer` -> `TileLayerComponent` (支持剔除渲染与对齐修正)。
        - **物理类型支持**: 支持 `SOLID` (碰撞)、`HAZARD` (伤害区域)、`LADDER` (可攀爬区域) 以及多种角度的 `SLOPE` (斜坡)。
    - `Object Layer`:
        - **视觉对象**: 将带 GID 的对象实例化为带 `Transform`, `Sprite` 组件的实体。
        - **形状对象 (GID=0)**: 支持加载矩形等形状，自动附加 `ColliderComponent` (触发器) 与静态 `PhysicsComponent`，用于实现关卡出口、检查点等逻辑触发区域。
- **路径解析**: 自动处理相对路径，确保纹理资源正确加载。

## 8. 视差滚动 (Parallax Scrolling)
- **ParallaxComponent**: 专门负责渲染背景图层的组件。
- **视差因子 (Factor)**: 通过 `parallax_factor_` 控制背景随相机移动的速度（例如 `0.2` 表示背景移动速度是相机的 0.2 倍，产生远景效果）。
- **Offset 支持**: 正确处理 Tiled 中的 `offsetx/offsety` 偏移量。

## 9. 动画系统 (Animation System)
- **基于时间的播放**: 动画逻辑与帧率无关。`AnimationComponent` 通过累计 `delta_time` 并在 `Animation` 对象中检索对应时刻的 `AnimationFrame`。
- **循环控制**: 
    - 循环动画使用 `fmod` 处理溢出时间，确保循环逻辑无漂移。
    - 非循环动画在到达 `total_duration` 后会自动停止并锁定在最后一帧，避免出现闪烁回到第一帧的 Bug。
- **数据驱动**: 动画数据可以直接定义在 Tiled 的 Tileset 属性中（JSON 字符串格式），由 `LevelLoader` 自动解析并挂载。
- **与 Sprite 交互**: `AnimationComponent` 每帧计算出当前的 `src_rect` 后，直接调用 `SpriteComponent::setSourceRect()` 更新渲染内容。

## 10. 物理系统 (Physics)

- **PhysicsEngine**: 维护所有 `PhysicsComponent` 的注册列表，并在每帧统一调用 `update(dt)`。
- **PhysicsComponent**: 作为组件挂载到 `GameObject` 上，内部持有速度、受力、质量等数据，并在 `init/clean` 时向 `PhysicsEngine` 注册/注销。

### 更新顺序

- 当前引擎实现中，`Scene::update(dt)` 会最先调用 `context_.getPhysicsEngine().update(dt)`，随后再更新每个 `GameObject` 的逻辑组件。

### 单位约定

- `Time::getDeltaTime()` 输出单位为 **秒 (s)**。
- `PhysicsEngine` 内的 `gravity_` 以 **像素/秒²** 表示（默认约 `980`）。
- `PhysicsComponent::velocity_` 以 **像素/秒** 表示。

### 当前限制（后续可扩展）

- 目前已实现 **基础碰撞检测**：`PhysicsEngine` 在 `update(dt)` 末尾执行 `checkObjectCollisions()`，对注册的物理对象两两检测碰撞。
- 碰撞基于 `ColliderComponent` + `engine::physics::collision::checkCollision()`：
  - 先做世界坐标 AABB 粗检（broad-phase）
  - 通过碰撞体类型再做细检（narrow-phase）：AABB/AABB、Circle/Circle、AABB/Circle
- 检测结果会写入 `PhysicsEngine::collision_pairs_`，并可通过 `PhysicsEngine::getCollisionPairs()` 读取（用于调试/测试）。

### 玩家掉落检测

- **实现位置**：在 `GameScene::update()` 方法中实现
- **检测逻辑**：检查玩家位置是否超出屏幕下方 100 像素
- **处理流程**：
  1. 获取玩家当前位置
  2. 获取视口大小
  3. 检查玩家位置.y 是否大于视口大小.y + 100.0f
  4. 如果超出，设置游戏失败状态，保存游戏数据，切换到结束场景
- **阈值设置**：100 像素的缓冲区，避免玩家因轻微超出屏幕就触发游戏失败

此外已实现 **瓦片碰撞分离（Tile collision resolve）**：

- `PhysicsEngine::update(dt)` 在对每个 `PhysicsComponent` 积分速度后调用 `resolveTileCollisions(pc, dt)`。
- `resolveTileCollisions()` 会根据物体的 `ColliderComponent` 世界 AABB 与 `TileLayerComponent` 的 SOLID 瓦片进行分离：
  - 采用 **分离轴 sweep**（先 X 后 Y）避免角落处左右方向表现不一致。
  - 采用对称 `eps`（很小值）对 AABB 边界向内缩，避免 `floor()` 带来的左右/上下取整不对称。
  - **斜坡物理增强**:
    - **自动吸附 (Snap/Stickiness)**: 在 `resolveYAxisCollision` 中实现了吸附逻辑，允许玩家在走下坡时保持贴地，避免因重力积分滞后导致的脱离地面和状态闪烁。
    - **即时状态更新**: 在水平移动（X轴）发生斜坡修正时，立即更新 `collided_below_` 标志，确保后续逻辑（如下一帧的初速度判定）正确。
    - **判定鲁棒性**: 选取检测范围内 Y 值最小（即物理高度最高）的斜坡面作为支撑面，彻底消除多重碰撞重叠时的“弹出”Bug。
    - **吸附禁用 (Snap Suppression)**: 提供的 `suppressSnapFor(seconds)` 方法允许状态机在起跳或特殊切出时暂时禁用吸附逻辑，防止被 Stickiness 拉回斜坡。
  - **梯子中心对齐**: 在进入攀爬（尤其是从上方边缘下落进入）时，引擎会自动计算梯子列的中心 X 坐标并将玩家吸附过去，确保动画展现的对齐。
  - 采样瓦片坐标时使用与 `TileLayerComponent::getTileTypeAtWorldPos()` 一致的世界偏移：`layer->getOffset() + layer_owner->Transform.position`，避免渲染与碰撞坐标系不一致。
  - 发生碰撞时将对应轴速度分量置零（例如撞墙清零 `velocity_.x`，落地清零 `velocity_.y`）。

## 每个场景的独特系统 (Scene Specific Systems)

### 1. 相机平滑跟随 (Smooth Camera Follow)
- **目标绑定**: `Camera` 支持绑定一个 `TransformComponent` 作为 `target_`。
- **平滑插值**: 使用 `glm::mix` (Lerp) 实现基于时间的平滑位移，计算公式为 `current = mix(current, target, factor * dt)`，有效滤除角色快速运动引起的视觉抖动。
- **边界约束**: 使用 `clampPosition` 确保相机不会露出地图黑边。

### 调试用例（GameScene）

`game::scene::GameScene` 内置了一个简单的碰撞测试用例（便于快速验证碰撞检测链路）：

- `test_object`：受重力影响的箱子，带 `AABBCollider(32x32)`
- `test_object2`：静止物体，不受重力，带 `CircleCollider(radius=16)`

在 `GameScene::update()` 中会调用 `TestCollisionPairs()`，每帧遍历并打印 `PhysicsEngine::getCollisionPairs()`。

## 11. 玩家状态系统详解 (Player State System)

玩家状态系统采用 **状态模式** 与 **命令模式** 结合的设计，实现灵活的玩家行为控制。

### 状态转换图

```mermaid
stateDiagram-v2
    [*] --> Idle: 初始化
    Idle --> Walk: moveLeft/moveRight
    Walk --> Idle: stopMove
    Idle --> Jump: jump
    Walk --> Jump: jump
    Jump --> Fall: 速度向下
    Fall --> Idle: 落地
    Idle --> Climb: climbUp (在梯子旁)
    Walk --> Climb: climbUp (在梯子旁)
    Fall --> Climb: climbUp (在梯子旁)
    Climb --> Idle: 离开梯子
    Idle --> Hurt: 受到伤害
    Walk --> Hurt: 受到伤害
    Jump --> Hurt: 受到伤害
    Fall --> Hurt: 受到伤害
    Climb --> Hurt: 受到伤害
    Hurt --> Idle: 受伤结束
    Hurt --> Dead: 生命值归零
    Idle --> Dead: 生命值归零
    Walk --> Dead: 生命值归零
    Jump --> Dead: 生命值归零
    Fall --> Dead: 掉落深渊
    Dead --> [*]: 游戏结束
```

### 各状态行为

#### IdleState (待机状态)
- **进入动作**: 播放待机动画，重置水平速度
- **支持动作**:
  - `moveLeft/moveRight`: 切换到 WalkState
  - `jump`: 切换到 JumpState
  - `climbUp`: 如果在梯子旁，切换到 ClimbState
- **更新逻辑**: 检查是否处于地面，检测掉落

#### WalkState (行走状态)
- **进入动作**: 播放行走动画
- **支持动作**:
  - `stopMove`: 切换到 IdleState
  - `jump`: 切换到 JumpState
- **更新逻辑**: 应用水平移动速度，检测掉落

#### JumpState (跳跃状态)
- **进入动作**: 应用跳跃冲量，播放跳跃动画
- **支持动作**:
  - `moveLeft/moveRight`: 空中水平移动控制
- **更新逻辑**: 检测速度方向，速度向下时切换到 FallState

#### FallState (下落状态)
- **进入动作**: 播放下落动画
- **支持动作**:
  - `moveLeft/moveRight`: 空中水平移动控制
  - `climbUp`: 如果在梯子旁，切换到 ClimbState
- **更新逻辑**: 检测落地，检测梯子

#### ClimbState (攀爬状态)
- **进入动作**: 播放攀爬动画，禁用重力
- **支持动作**:
  - `climbUp/climbDown`: 垂直移动
  - `stopMove`: 停止攀爬动画
- **更新逻辑**: 检测是否离开梯子，离开时切换到 IdleState

#### HurtState (受伤状态)
- **进入动作**: 播放受伤动画，应用击退力
- **支持动作**: 限制大部分动作
- **更新逻辑**: 倒计时受伤时间，结束后切换到 IdleState

#### DeadState (死亡状态)
- **进入动作**: 播放死亡动画
- **支持动作**: 不响应任何输入
- **更新逻辑**: 触发游戏结束

### 动作接口实现

每个状态类通过实现动作接口来控制状态转换：

```cpp
// IdleState 示例
std::unique_ptr<PlayerState> IdleState::moveLeft(engine::core::Context& context) {
    return std::make_unique<WalkState>(player_);
}

std::unique_ptr<PlayerState> IdleState::jump(engine::core::Context& context) {
    if (auto* physics = player_.getPhysicsComponent()) {
        physics->addVelocity(glm::vec2(0.0f, -jump_force));
    }
    return std::make_unique<JumpState>(player_);
}

// 不支持的动作返回 nullptr
std::unique_ptr<PlayerState> IdleState::attack(engine::core::Context& context) {
    return nullptr;  // 待机状态不处理攻击
}
```

### 状态切换机制

```cpp
void PlayerComponent::changeState(std::unique_ptr<PlayerState> new_state) {
    if (!new_state || new_state.get() == state_.get()) {
        return;
    }
    
    // 退出当前状态
    state_->exit(context_);
    
    // 切换状态
    state_ = std::move(new_state);
    
    // 进入新状态
    state_->enter(context_);
}
```

## 12. 游戏状态管理 (GameState)

`GameState` 类负责管理游戏的全局状态，包括游戏运行状态、窗口尺寸等信息。

### 状态类型

```cpp
enum class GameStateType {
    Title,      // 标题场景
    Playing,    // 游戏进行中
    Paused,     // 游戏暂停
    GameOver,   // 游戏结束
};
```

### 核心功能

#### 状态管理
- `getState()`: 获取当前游戏状态
- `setState(GameStateType)`: 设置游戏状态
- `isPlaying()`: 检查是否处于游戏进行中状态
- `isPaused()`: 检查是否处于暂停状态
- `isGameOver()`: 检查是否处于游戏结束状态

#### 窗口管理
- `getWindowSize()`: 获取窗口物理尺寸（像素）
- `setWindowSize(vec2)`: 设置窗口物理尺寸
- `getWindowLogicalSize()`: 获取窗口逻辑尺寸（用于渲染坐标系）
- `setWindowLogicalSize(vec2)`: 设置窗口逻辑尺寸

### 状态转换流程

```mermaid
stateDiagram-v2
    [*] --> Title: 游戏启动
    Title --> Playing: 开始游戏
    Playing --> Paused: 按下暂停键
    Paused --> Playing: 恢复游戏
    Playing --> GameOver: 游戏失败/胜利
    GameOver --> Title: 返回主菜单
    GameOver --> Playing: 重新开始
    Paused --> Title: 返回主菜单
```

### 使用场景

1. **场景切换控制**: `SceneManager` 根据 `GameState` 决定是否处理场景更新
2. **输入响应**: `InputManager` 根据状态决定如何处理输入（如暂停时忽略游戏输入）
3. **UI 显示**: UI 系统根据状态显示不同的界面（暂停菜单、游戏结束画面等）
4. **窗口管理**: 提供统一的窗口尺寸查询接口，支持不同分辨率的适配

## 13. 脏标识模式 (Dirty Flag Pattern)

本项目使用 **脏标识模式** 优化 UI 文本渲染性能，避免每帧执行昂贵的文本测量和渲染状态同步操作。

### 13.1 问题背景

文本渲染涉及多个昂贵操作：
- **尺寸测量**: `TTF_GetTextSize` 需要计算字形布局
- **字体切换**: `TTF_SetTextFont` 可能触发纹理更新
- **文本更新**: `TTF_SetTextString` 需要重新准备字形

如果这些操作每帧都执行，会造成严重的性能瓶颈。

### 13.2 两层脏标识架构

```mermaid
flowchart TB
    subgraph UI层["UI层 (UIText)"]
        A[text_/font_path_/font_size_ 变化] --> B{is_dirty_}
        B -->|true| C[ensureUpToDate]
        B -->|false| D[使用缓存值]
        C --> E[updateSize]
    end
    
    subgraph 渲染层["渲染层 (TextRenderer)"]
        E --> F[getTextSize/drawUIText]
        F --> G{is_dirty 参数}
        G -->|true| H[同步TTF_Text状态<br/>TTF_SetTextFont<br/>TTF_SetTextString]
        G -->|false| I[直接复用缓存]
    end
    
    style UI层 fill:#e3f2fd
    style 渲染层 fill:#fff3e0
```

### 13.3 UI层实现 (UIText)

**文件**: `src/engine/ui/ui_text.h/cpp`

```cpp
class UIText : public UIElement {
private:
    std::string text_;           // 文本内容
    std::string font_path_;      // 字体路径
    int font_size_ = 16;         // 字体大小
    bool is_dirty_ = true;       // 脏标识
    
public:
    void setText(const std::string& text) {
        text_ = text;
        is_dirty_ = true;        // 标记脏，不立即计算
    }
    
    const glm::vec2& getSize() const override {
        ensureUpToDate();        // 访问时检查
        return size_;
    }
    
    void render() override {
        ensureUpToDate();
        text_renderer.drawUIText(text_, ..., is_dirty_);
        is_dirty_ = false;       // 渲染后清除脏标记
    }
    
private:
    void ensureUpToDate() const {
        if (is_dirty_) {
            const_cast<UIText*>(this)->updateSize();
        }
    }
    
    void updateSize() {
        size_ = text_renderer.getTextSize(text_, font_path_, font_size_, is_dirty_);
    }
};
```

**设计要点**:
- **延迟计算**: Setter 只标记脏状态，不立即执行昂贵操作
- **访问时计算**: 在 `getSize()` 和 `render()` 时才检查并刷新
- **自动传播**: 将脏状态传递给渲染层

### 13.4 渲染层实现 (TextRenderer)

**文件**: `src/engine/render/text_renderer.h/cpp`

```cpp
class TextRenderer {
public:
    // 绘制UI文本，is_dirty 决定是否执行昂贵同步
    void drawUIText(const std::string& text,
                   const std::string& font_path,
                   int font_size,
                   const glm::vec2& position,
                   const FColor& color,
                   bool is_dirty = true);
    
    // 获取文本尺寸
    glm::vec2 getTextSize(const std::string& text,
                          const std::string& font_path,
                          int font_size,
                          bool is_dirty);
    
private:
    // 缓存：key 是 string 对象的地址
    std::unordered_map<std::uintptr_t, std::unique_ptr<TTF_Text, TTFTextDeleter>> text_cache_;
};
```

**实现逻辑**:

```cpp
void TextRenderer::drawUIText(..., bool is_dirty) {
    TTF_Text* ttf_text = nullptr;
    
    if (is_dirty) {
        // 脏状态：执行昂贵同步
        ttf_text = getTTFText(text);
        if (!ttf_text) {
            ttf_text = createTTFText(text, font);
        }
        TTF_SetTextFont(ttf_text, font);      // 昂贵操作
        TTF_SetTextString(ttf_text, text.c_str(), 0);  // 昂贵操作
    } else {
        // 干净状态：直接复用缓存
        ttf_text = getTTFText(text);
    }
    
    // 绘制（无论脏/干净都执行）
    TTF_DrawRendererText(ttf_text, position.x, position.y);
}
```

### 13.5 缓存策略

**缓存Key设计**:
- 使用 `std::uintptr_t(&text)` 即 string 对象的地址作为 key
- **优点**: O(1) 查找，无需字符串哈希计算
- **适用场景**: UI 文本对象生命周期稳定

```cpp
TTF_Text* TextRenderer::getTTFText(const std::string& text) {
    const auto cache_key = reinterpret_cast<std::uintptr_t>(&text);
    auto it = text_cache_.find(cache_key);
    if (it == text_cache_.end()) {
        return nullptr;
    }
    return it->second.get();
}
```

### 13.6 典型使用场景

#### 场景A：创建UI后立即布局

```cpp
// EndScene::createUI()
auto title_label = std::make_unique<UIText>(context_, "Game Over", font_path, 48);
// 此时 is_dirty_ = true

// 需要计算居中位置
glm::vec2 text_size = title_label->getSize();  // 触发 ensureUpToDate()
// 内部调用 TextRenderer::getTextSize(..., true)
// 创建 TTF_Text 并同步状态

float center_x = (window_size.x - text_size.x) / 2.0f;
title_label->setPosition(center_x, 100.0f);
```

#### 场景B：分数每帧渲染但很少变化

```cpp
// 得分时触发变化
void UIText::onNotify(EventType event_type, const std::any& data) {
    if (event_type == EventType::SCORE_CHANGED) {
        if (const int* score = std::any_cast<int>(&data)) {
            setText("Score: " + std::to_string(*score));  // is_dirty_ = true
        }
    }
}

// 渲染循环
void UIText::render() {
    ensureUpToDate();  // 只有脏时才更新 size_
    
    text_renderer.drawUIText(text_, ..., is_dirty_);
    // is_dirty_ == true:  执行 TTF_SetText* (昂贵)
    // is_dirty_ == false: 直接绘制缓存 (廉价)
    
    is_dirty_ = false;  // 清除脏标记
}
```

### 13.7 性能收益

| 操作 | 每帧执行（无脏标识） | 变化时执行（有脏标识） | 收益 |
|------|---------------------|----------------------|------|
| `TTF_SetTextFont` | ✓ | 仅字体变化时 | 避免重复设置 |
| `TTF_SetTextString` | ✓ | 仅文本变化时 | 避免重复准备字形 |
| `getTextSize` | ✓ | 仅脏时 | 避免重复测量 |
| 纹理更新 | ✓ | 仅必要时 | 显著减少GPU上传 |

### 13.8 生命周期安全

缓存的 `TTF_Text` 必须在 SDL_ttf 仍有效时销毁：

```cpp
TextRenderer::~TextRenderer() {
    // 1. 先清空缓存（销毁所有 TTF_Text）
    text_cache_.clear();
    
    // 2. 再销毁 text_engine_
    if (text_engine_) {
        TTF_DestroyRendererTextEngine(text_engine_);
    }
    
    // 3. 最后才 TTF_Quit()
    --text_renderer_instances;
    if (text_renderer_instances <= 0) {
        TTF_Quit();
    }
}
```

**关键点**: 严格的析构顺序避免退出时崩溃。

### 13.9 设计优势

1. **分层解耦**: UI层负责标记变化，渲染层负责优化执行
2. **延迟计算**: 避免不必要的中间计算
3. **缓存复用**: 稳定的 UI 元素享受最大性能收益
4. **自动管理**: 脏状态自动传播和清除，使用简单