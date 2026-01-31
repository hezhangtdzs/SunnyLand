# 开发规范 (Development Guidelines)

## 1. 创建新场景
- 继承 `engine::scene::Scene`。
- 在 `init()` 中使用 `createGameObject` 或 `std::make_unique` 构建初始化实体。
- 通过 `scene_manager_.requestReplaceScene()` 实现场景跳转。
- 对于结束场景 (EndScene)，需要：
  - 在构造函数中处理 `SessionData` 的获取和初始化
  - 在 `createUI()` 方法中构建完整的结束界面
  - 实现游戏胜利/失败状态的显示逻辑
  - 提供重新开始和返回主菜单的功能

## 2. 创建新组件
- 继承 `engine::component::Component`。
- 逻辑代码写在 `update()` 中，渲染代码写在 `render()` 中。
- 获取同对象的其他组件请使用 `owner_->getComponent<T>()`。

## 3. 使用 PhysicsComponent（示例）

- 给对象添加物理：`addComponent<PhysicsComponent>(&context_.getPhysicsEngine())`
- 关闭重力：`setUseGravity(false)`
- 施加一次性力：`addForce(...)`（会在该帧计算完后清空）
- 直接设置速度：`setVelocity(...)`（用于跳跃/冲量类效果）

## 4. 使用音频系统（示例）

### 方式 A：组件内按 action/id 播放

- 给对象添加音频组件：`addComponent<AudioComponent>()`
- 注册音效映射：`registerSound("jump", "assets/sounds/jump.wav")`
- 设置节流（可选）：`setMinIntervalMs(80)`
- 触发播放：`playSound("jump", context)`

### 方式 B：直接播放路径

- 直接播放某个文件：`context.getAudioPlayer().playSound("assets/sounds/coin.wav")`
- 播放 BGM：`context.getAudioPlayer().playMusic("assets/music/level1.ogg")`

## 5. 使用 SessionData（示例）

### 5.1 初始化和获取 SessionData

```cpp
// 初始化 SessionData（通常在 GameApp 中）
auto session_data = game::data::SessionData::getInstance();
// 加载存档数据
session_data->load();
// 设置初始关卡路径
session_data->setMapPath("assets/maps/level1.tmj");
```

### 5.2 获取和修改游戏状态

```cpp
// 获取 SessionData 实例
auto session_data = game::data::SessionData::getInstance();

// 获取游戏状态
int current_health = session_data->getCurrentHealth();
int current_score = session_data->getCurrentScore();
bool is_win = session_data->getIsWin();

// 修改游戏状态
session_data->setCurrentHealth(3);
session_data->setCurrentScore(current_score + 100);
session_data->setIsWin(true); // 设置游戏胜利状态

// 增加得分（推荐使用）
session_data->addScore(50); // 增加 50 分
```

### 5.3 保存和加载游戏数据

```cpp
// 保存游戏数据
auto session_data = game::data::SessionData::getInstance();
session_data->save();

// 加载游戏数据
session_data->load();
```

## 6. 关卡切换时的数据传递（示例）

```cpp
// 在 GameScene 中处理关卡切换
if (player->getName() == "player" && (trigger->getTag() == "next_level" || trigger->getName() == "win")) {
    // 获取 SessionData 实例
    auto session_data = game::data::SessionData::getInstance();
    
    // 保存当前游戏状态
    session_data->save();
    
    // 设置下一关路径
    std::string next_level_path = "assets/maps/" + trigger->getName() + ".tmj";
    session_data->setMapPath(next_level_path);
    
    // 创建新场景并传入会话数据
    auto next_scene = std::make_unique<GameScene>(
        "GameScene", 
        context_, 
        scene_manager_, 
        session_data,
        next_level_path);
    scene_manager_.requestReplaceScene(std::move(next_scene));
}
```

## 7. 性能优化
- **避免在每帧 `update` 中分配内存**: 尽量复用对象。
- **批量渲染**: 相同纹理的对象尽量连续渲染（后续计划）。
- **资源预加载**: 在 `Scene::init()` 中一次性加载场景所需资源。
- **UI 渲染优化**: 对于静态 UI 元素，考虑使用批处理渲染减少绘制调用。
- **状态机优化**: 状态切换逻辑应该简单明了，避免在状态处理中执行复杂计算。
- **精灵管理**: 对于频繁切换的精灵，考虑使用精灵图集减少纹理切换开销。

## 8. UI 系统开发规范

### 8.1 创建交互式 UI 元素
- 继承 `UIInteractive` 类实现自定义交互式元素。
- 为不同状态创建对应的精灵和音效。
- 使用状态模式管理不同的交互状态。

### 8.2 创建按钮
- 使用 `UIButton` 类创建标准按钮。
- 对于精灵按钮，提供正常、悬停、按下三种状态的精灵路径。
- 使用 Lambda 表达式或函数对象设置点击回调。

### 8.3 UI 布局最佳实践
- **使用相对坐标**: 子元素位置相对于父元素，便于整体移动和缩放。
- **分层设计**: 使用 `UIPanel` 作为容器组织相关 UI 元素。
- **响应式布局**: 考虑不同屏幕尺寸的适配（后续计划）。
- **输入处理**: 确保交互式元素正确处理输入事件，避免事件穿透。

### 8.4 资源管理
- **精灵资源**: 按钮精灵应该尺寸一致，便于布局管理。
- **音效资源**: 为不同状态选择合适的音效，增强用户体验。

## 9. 游戏失败检测和场景切换

### 9.1 游戏失败检测
- **生命值检测**: 在 `GameScene::update()` 中检查玩家生命值是否耗尽
- **掉落检测**: 检查玩家位置是否超出屏幕下方，建议设置 100 像素的缓冲区
- **其他失败条件**: 根据游戏设计添加其他失败条件，如时间耗尽等

### 9.2 场景切换最佳实践
- **状态保存**: 在切换场景前保存游戏状态，确保数据持久化
- **状态设置**: 设置游戏胜利/失败状态，用于结束场景的显示
- **错误处理**: 处理 `SessionData` 为空的情况，确保场景切换的安全性
- **调试日志**: 添加详细的调试日志，记录场景切换的原因和过程

### 9.3 实现示例

```cpp
// 游戏失败检测和场景切换示例
if (session_data->getCurrentHealth() <= 0) {
    spdlog::info("玩家生命值耗尽，游戏失败！");
    session_data->setIsWin(false);
    session_data->save();
    
    auto end_scene = std::make_unique<EndScene>(
        context_, 
        scene_manager_, 
        session_data);
    scene_manager_.requestReplaceScene(std::move(end_scene));
    return;
}

// 玩家掉落检测示例
if (player) {
    auto* transform = player->getComponent<engine::component::TransformComponent>();
    if (transform) {
        auto position = transform->getPosition();
        auto viewport_size = context_.getCamera().getViewportSize();
        if (position.y > viewport_size.y + 100.0f) {
            spdlog::info("玩家掉出屏幕，游戏失败！");
            session_data->setIsWin(false);
            session_data->save();
            
            auto end_scene = std::make_unique<EndScene>(
                context_, 
                scene_manager_, 
                session_data);
            scene_manager_.requestReplaceScene(std::move(end_scene));
            return;
        }
    }
}
```

## 10. 使用生成器模式（示例）

### 10.1 从头构建新对象

```cpp
#include "game/object/game_object_builder.h"

// 创建生成器
GameObjectBuilder builder(level_loader, context);

// 构建敌人对象
auto enemy = builder
    .configure(&object_json, &tile_json, tile_info)
    .setEnemyType("eagle")  // 设置敌人类型
    .build()
    .getGameObject();

// 将对象添加到场景
scene->safeAddGameObject(std::move(enemy));
```

### 10.2 使用自动类型检测

```cpp
GameObjectBuilder builder(level_loader, context);

// 根据对象名称自动推断类型
auto game_object = builder
    .configure(&object_json, &tile_json, tile_info)
    .autoDetectType(object_name)  // 自动识别 player/eagle/frog 等
    .build()
    .getGameObject();

scene->safeAddGameObject(std::move(game_object));
```

### 10.3 增强已有对象

```cpp
GameObjectBuilder builder(level_loader, context);

// 为已有对象添加游戏特定组件
builder.autoDetectType("frog")
       .enhance(existing_object)
       .buildEnhancement();

// 现在 existing_object 已经添加了 JumpBehavior AI
```

### 10.4 支持的类型映射

| 名称 | 类型 | 自动添加的组件 |
|:---|:---|:---|
| "player" | 玩家 | PlayerComponent + 状态机 |
| "eagle" | 敌人 | AIComponent + UpDownBehavior |
| "frog" | 敌人 | AIComponent + JumpBehavior |
| "opossum" | 敌人 | AIComponent + PatrolBehavior |
| "fruit" | 道具 | 动画播放 + "item" 标签 |
| "gem" | 道具 | 动画播放 + "item" 标签 |

## 11. 使用 GameState（示例）

### 11.1 初始化 GameState

```cpp
// 在 GameApp 中初始化
auto game_state = std::make_unique<engine::core::GameState>(
    renderer, 
    window, 
    engine::core::GameStateType::Title
);
```

### 11.2 状态切换

```cpp
// 开始游戏
void TitleScene::onStartClicked() {
    context_.getGameState().setState(engine::core::GameStateType::Playing);
    scene_manager_.requestReplaceScene(std::make_unique<GameScene>(...));
}

// 暂停游戏
void GameScene::handleInput() {
    if (input_manager.isActionPressed("pause")) {
        if (game_state_.isPlaying()) {
            game_state_.setState(engine::core::GameStateType::Paused);
            scene_manager_.requestPushScene(std::make_unique<MenuScene>(...));
        }
    }
}

// 恢复游戏
void MenuScene::onResumeClicked() {
    context_.getGameState().setState(engine::core::GameStateType::Playing);
    scene_manager_.requestPopScene();
}
```

### 11.3 根据状态处理逻辑

```cpp
void GameScene::update(float delta_time) {
    // 只有在游戏进行中状态才更新游戏逻辑
    if (!context_.getGameState().isPlaying()) {
        return;
    }
    
    // 更新物理
    context_.getPhysicsEngine().update(delta_time);
    
    // 更新游戏对象
    for (auto& game_object : game_objects_) {
        game_object->update(delta_time, context_);
    }
}
```

### 11.4 窗口尺寸查询

```cpp
// 获取窗口逻辑尺寸（用于 UI 布局）
auto window_size = context_.getGameState().getWindowLogicalSize();
float center_x = window_size.x / 2.0f;
float center_y = window_size.y / 2.0f;

// 创建居中的 UI 元素
auto button = std::make_unique<UIButton>(
    context_,
    normal_sprite,
    hover_sprite,
    pressed_sprite,
    glm::vec2(center_x - button_width / 2, center_y - button_height / 2),
    glm::vec2(button_width, button_height),
    callback
);
```

## 12. 使用命令模式（示例）

### 12.1 初始化命令映射器

```cpp
#include "game/command/command_mapper.h"
#include "game/command/player_commands.h"

void GameScene::initCommandMapper() {
    command_mapper_ = std::make_unique<game::command::CommandMapper>();

    // 绑定动作到命令
    command_mapper_->bind("move_left", 
        std::make_unique<game::command::MoveLeftCommand>(player_component_));
    command_mapper_->bind("move_right", 
        std::make_unique<game::command::MoveRightCommand>(player_component_));
    command_mapper_->bind("jump", 
        std::make_unique<game::command::JumpCommand>(player_component_));
    command_mapper_->bind("attack", 
        std::make_unique<game::command::AttackCommand>(player_component_));
    command_mapper_->bind("climb_up", 
        std::make_unique<game::command::ClimbUpCommand>(player_component_));
    command_mapper_->bind("climb_down", 
        std::make_unique<game::command::ClimbDownCommand>(player_component_));
    command_mapper_->bind("stop_move", 
        std::make_unique<game::command::StopMoveCommand>(player_component_));
}
```

### 12.2 在输入处理中使用命令

```cpp
bool GameScene::handleInput() {
    Scene::handleInput();
    auto& input_manager = context_.getInputManager();

    // 处理移动输入
    if (input_manager.isActionDown("move_left")) {
        command_mapper_->execute("move_left", context_);
    }
    else if (input_manager.isActionDown("move_right")) {
        command_mapper_->execute("move_right", context_);
    }
    else {
        command_mapper_->execute("stop_move", context_);
    }

    // 处理跳跃输入（按下触发）
    if (input_manager.isActionPressed("jump")) {
        command_mapper_->execute("jump", context_);
    }

    // 处理攻击输入（按下触发）
    if (input_manager.isActionPressed("attack")) {
        command_mapper_->execute("attack", context_);
    }

    return true;
}
```

### 12.3 创建自定义命令

```cpp
#include "game/command/player_commands.h"

// 继承 PlayerCommand 基类
class DashCommand : public game::command::PlayerCommand {
    game::component::PlayerComponent* player_;
public:
    explicit DashCommand(game::component::PlayerComponent* player) : player_(player) {}
    
    void execute(engine::core::Context& context) override {
        if (player_) {
            // 实现冲刺逻辑
            player_->dash(context);
        }
    }
};

// 在 PlayerComponent 中添加 dash 方法
void PlayerComponent::dash(engine::core::Context& context) {
    if (!current_state_) return;
    auto new_state = current_state_->dash(context);
    if (new_state) {
        setState(std::move(new_state));
    }
}
```

### 12.4 实现双人控制切换

```cpp
void GameScene::switchPlayer() {
    // 查找第二个玩家
    auto* player2 = findGameObjectByName("player2");
    if (!player2) return;

    // 切换当前控制玩家
    current_controlled_player_ = (current_controlled_player_ == player_) 
        ? player2 : player_;

    // 切换相机跟随
    auto* transform = current_controlled_player_->getComponent<TransformComponent>();
    context_.getCamera().setTarget(transform);

    // 重新绑定命令到新玩家
    auto* new_player_component = current_controlled_player_
        ->getComponent<game::component::PlayerComponent>();
    if (new_player_component) {
        rebindCommandMapper(new_player_component);
    }
}

void GameScene::rebindCommandMapper(game::component::PlayerComponent* player_component) {
    // 清除现有命令
    command_mapper_->clear();
    
    // 重新绑定所有命令到新玩家
    command_mapper_->bind("move_left", 
        std::make_unique<game::command::MoveLeftCommand>(player_component));
    command_mapper_->bind("move_right", 
        std::make_unique<game::command::MoveRightCommand>(player_component));
    command_mapper_->bind("jump", 
        std::make_unique<game::command::JumpCommand>(player_component));
    command_mapper_->bind("attack", 
        std::make_unique<game::command::AttackCommand>(player_component));
    command_mapper_->bind("climb_up", 
        std::make_unique<game::command::ClimbUpCommand>(player_component));
    command_mapper_->bind("climb_down", 
        std::make_unique<game::command::ClimbDownCommand>(player_component));
    command_mapper_->bind("stop_move", 
        std::make_unique<game::command::StopMoveCommand>(player_component));
}
```

### 12.5 命令模式最佳实践

1. **单一职责**: 每个命令类只负责一个具体的动作
2. **空检查**: 在 execute() 中始终检查 player_ 指针是否为空
3. **资源管理**: 使用 unique_ptr 管理命令对象，避免内存泄漏
4. **动态绑定**: 利用 CommandMapper 的动态绑定特性，支持运行时切换
5. **输入映射**: 在 config.h 中定义新的输入动作，保持一致性

### 12.6 支持的命令列表

| 命令类 | 功能 | 使用场景 |
|:---|:---|:---|
| `MoveLeftCommand` | 向左移动 | 按住 A 或 Left 键 |
| `MoveRightCommand` | 向右移动 | 按住 D 或 Right 键 |
| `JumpCommand` | 跳跃 | 按下 J 或 Space 键 |
| `AttackCommand` | 攻击 | 按下 K 或鼠标左键 |
| `ClimbUpCommand` | 向上攀爬 | 在梯子上按 W 或 Up 键 |
| `ClimbDownCommand` | 向下攀爬 | 在梯子上按 S 或 Down 键 |
| `StopMoveCommand` | 停止移动 | 无移动输入时自动触发 |