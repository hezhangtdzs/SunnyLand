# API 参考 (API Reference)

本文档提供 SunnyLand 项目核心 API 的快速参考。

## 目录

- [GameObject](#gameobject)
- [Component](#component)
- [Scene](#scene)
- [Context](#context)
- [InputManager](#inputmanager)
- [AudioLocator](#audiolocator)
- [SessionData](#sessiondata)
- [PhysicsEngine](#physicsengine)

---

## GameObject

游戏对象基类，采用组件化架构。

### 头文件
`#include "engine/object/game_object.h"`

### 构造函数

```cpp
GameObject(const std::string& name = "", const std::string& tag = "");
```

### 模板方法

#### addComponent
```cpp
template<typename T, typename... Args>
T* addComponent(Args&&... args);
```
为游戏对象添加组件。

**示例**:
```cpp
auto* physics = game_object->addComponent<PhysicsComponent>(&physics_engine);
auto* sprite = game_object->addComponent<SpriteComponent>(texture_id);
```

#### getComponent
```cpp
template<typename T>
T* getComponent() const;
```
获取指定类型的组件。

**示例**:
```cpp
auto* transform = game_object->getComponent<TransformComponent>();
auto* physics = game_object->getComponent<PhysicsComponent>();
```

#### hasComponent
```cpp
template<typename T>
bool hasComponent() const;
```
检查对象是否包含特定类型的组件。

**示例**:
```cpp
if (game_object->hasComponent<HealthComponent>()) {
    // 处理生命值相关逻辑
}
```

#### removeComponent
```cpp
template<typename T>
void removeComponent();
```
移除指定类型的组件。

### 属性方法

```cpp
const std::string& getName() const;
void setName(const std::string& name);
const std::string& getTag() const;
void setTag(const std::string& tag);
void setNeedRemove(bool need_remove);
bool getNeedRemove() const;
```

---

## Component

组件基类，定义了游戏对象组件的基础接口。

### 头文件
`#include "engine/component/component.h"`

### 生命周期方法

```cpp
virtual void init();           // 组件初始化
virtual void handleInput(Context& context);  // 处理输入
virtual void update(float deltaTime, Context& context);  // 更新逻辑
virtual void render(Context& context);  // 渲染
virtual void clean();          // 清理资源
```

### 属性方法

```cpp
void setOwner(GameObject* owner);
GameObject* getOwner() const;
```

---

## Scene

场景基类，管理游戏对象和 UI。

### 头文件
`#include "engine/scene/scene.h"`

### 核心方法

```cpp
virtual void init();           // 初始化场景
virtual void enter();          // 进入场景
virtual void exit();           // 退出场景
virtual void update(float delta_time);  // 更新场景
virtual void render();         // 渲染场景
virtual void handleInput();    // 处理输入
virtual void clean();          // 清理场景
```

### 对象管理

```cpp
void safeAddGameObject(std::unique_ptr<GameObject> game_object);
void safeRemoveGameObject(GameObject* game_object);
GameObject* findGameObjectByName(const std::string& name);
std::vector<GameObject*> findGameObjectsByTag(const std::string& tag);
```

### 访问 Context

```cpp
engine::core::Context& getContext() const;
```

---

## Context

游戏引擎上下文类，集中管理引擎的核心系统。

### 头文件
`#include "engine/core/context.h"`

### 获取系统引用

```cpp
engine::render::Renderer& getRenderer();
engine::render::TextRenderer& getTextRenderer();
engine::render::Camera& getCamera();
engine::resource::ResourceManager& getResourceManager();
engine::input::InputManager& getInputManager();
engine::physics::PhysicsEngine& getPhysicsEngine();
engine::core::GameState& getGameState();
```

**示例**:
```cpp
// 在组件中使用 Context
void MyComponent::update(float delta_time, Context& context) {
    auto& input = context.getInputManager();
    auto& physics = context.getPhysicsEngine();
    
    if (input.isActionPressed("jump")) {
        // 处理跳跃
    }
}
```

---

## InputManager

输入管理器，处理键盘和鼠标输入。

### 头文件
`#include "engine/input/input_manager.h"`

### 核心方法

```cpp
void Update();  // 每帧更新输入状态

// 动作检测
bool isActionPressed(const std::string& action);  // 按下触发（一次性）
bool isActionDown(const std::string& action);     // 按住状态
bool isActionReleased(const std::string& action); // 释放触发（一次性）

// 原始输入
bool shouldQuit() const;  // 是否请求退出
```

### 配置动作映射

动作映射在 `assets/config.json` 中配置：

```json
{
    "input_mappings": {
        "move_left": ["A", "Left"],
        "move_right": ["D", "Right"],
        "jump": ["J", "Space"],
        "attack": ["K", "MouseLeft"]
    }
}
```

**示例**:
```cpp
void PlayerComponent::update(float delta_time, Context& context) {
    auto& input = context.getInputManager();
    
    if (input.isActionDown("move_left")) {
        moveLeft();
    }
    
    if (input.isActionPressed("jump")) {
        jump();
    }
}
```

---

## AudioLocator

服务定位器，提供全局访问音频系统的能力。

### 头文件
`#include "engine/audio/audio_locator.h"`

### 静态方法

```cpp
// 获取音频服务
static IAudioPlayer& get();

// 注册音频服务
static void provide(IAudioPlayer* service);
```

### 使用示例

```cpp
// 播放音效
engine::audio::AudioLocator::get().playSound("assets/audio/jump.wav");

// 播放背景音乐
engine::audio::AudioLocator::get().playMusic("assets/audio/bgm.ogg", -1);

// 停止音乐
engine::audio::AudioLocator::get().stopMusic();

// 设置音量
engine::audio::AudioLocator::get().setMasterVolume(0.8f);
engine::audio::AudioLocator::get().setSoundVolume(0.5f);
engine::audio::AudioLocator::get().setMusicVolume(0.7f);
```

---

## SessionData

游戏会话数据管理类，跨场景持久化游戏状态。

### 头文件
`#include "game/data/session_data.h"`

### 单例获取

```cpp
static std::shared_ptr<SessionData> getInstance(
    int max_health = 3,
    const std::string& initial_map_path = "assets/maps/level1.tmj",
    const std::string& save_file_path = "assets/save_data.json"
);
```

### 数据管理

```cpp
// 保存/加载
bool save() const;
bool load();
void reset();

// 生命值
int getCurrentHealth() const;
void setCurrentHealth(int health);
int getMaxHealth() const;
void setMaxHealth(int max_health);

// 分数
int getCurrentScore() const;
void setCurrentScore(int score);
void addScore(int score);
int getHighScore() const;
bool updateHighScore();

// 关卡
const std::string& getMapPath() const;
void setMapPath(const std::string& map_path);

// 游戏状态
bool getIsWin() const;
void setIsWin(bool is_win);
```

### 使用示例

```cpp
// 获取实例
auto session_data = game::data::SessionData::getInstance();

// 加载存档
session_data->load();

// 增加分数
session_data->addScore(100);

// 保存游戏
session_data->save();
```

---

## PhysicsEngine

物理引擎，管理物理组件和碰撞检测。

### 头文件
`#include "engine/physics/physics_engine.h"`

### 核心方法

```cpp
// 更新物理
void update(float delta_time);

// 注册/注销物理组件
void registerPhysicsComponent(PhysicsComponent* physics_component);
void unregisterPhysicsComponent(PhysicsComponent* physics_component);

// 注册/注销瓦片层
void registerCollisionLayer(TileLayerComponent* tilelayer_component);
void unregisterCollisionLayer(TileLayerComponent* tilelayer_component);

// 获取碰撞信息
const auto& getCollisionPairs() const;
const auto& getTileTriggerEvents() const;

// 查询瓦片
TileType getTileTypeAt(const glm::vec2& world_pos) const;
bool tryGetLadderColumnCenterX(const glm::vec2& world_pos, float& out_center_x) const;
```

### 属性设置

```cpp
void setGravity(const glm::vec2& gravity);
const glm::vec2& getGravity() const;
void setMaxSpeed(float max_speed);
float getMaxSpeed() const;
void setWorldBounds(const engine::utils::Rect& bounds);
```

---

## PhysicsComponent

物理组件，为游戏对象添加物理属性。

### 头文件
`#include "engine/component/physics_component.h"`

### 核心方法

```cpp
// 力和速度
void addForce(const glm::vec2& force);
void setVelocity(const glm::vec2& velocity);
glm::vec2 getVelocity() const;
void clearForce();
glm::vec2 getForce() const;

// 属性
void setMass(float mass);
float getMass() const;
void setUseGravity(bool use_gravity);
bool isUseGravity() const;
void setEnabled(bool enable);
bool isEnabled() const;

// 攀爬
void setClimbing(bool climbing);
bool isClimbing() const;

// 碰撞标记
void resetCollisionFlags();
bool hasCollidedBelow() const;
bool hasCollidedAbove() const;
bool hasCollidedLeft() const;
bool hasCollidedRight() const;

// 吸附控制
void suppressSnapFor(float seconds);
bool isSnapSuppressed() const;
```

### 使用示例

```cpp
// 添加物理组件
auto* physics = game_object->addComponent<PhysicsComponent>(&physics_engine);

// 设置属性
physics->setMass(1.0f);
physics->setUseGravity(true);

// 施加力
physics->addForce(glm::vec2(100.0f, 0.0f));  // 向右推

// 直接设置速度（跳跃）
physics->setVelocity(glm::vec2(0.0f, -350.0f));  // 向上跳

// 检测碰撞
if (physics->hasCollidedBelow()) {
    // 在地面上
}
```

---

## 常用代码片段

### 创建游戏对象

```cpp
// 创建对象
auto game_object = std::make_unique<GameObject>("player", "hero");

// 添加组件
auto* transform = game_object->addComponent<TransformComponent>();
transform->setPosition(glm::vec2(100.0f, 200.0f));

game_object->addComponent<SpriteComponent>("player_texture");
game_object->addComponent<PhysicsComponent>(&physics_engine);

// 添加到场景
scene->safeAddGameObject(std::move(game_object));
```

### 场景切换

```cpp
// 切换场景
auto new_scene = std::make_unique<GameScene>(
    "GameScene", 
    context_, 
    scene_manager_
);
scene_manager_.requestReplaceScene(std::move(new_scene));

// 压入场景（如暂停菜单）
auto menu_scene = std::make_unique<MenuScene>(context_, scene_manager_);
scene_manager_.requestPushScene(std::move(menu_scene));

// 弹出场景（返回上一场景）
scene_manager_.requestPopScene();
```

### 播放动画

```cpp
// 添加动画组件
auto* anim = game_object->addComponent<AnimationComponent>();

// 创建动画
auto walk_anim = std::make_unique<Animation>("walk", true);
walk_anim->addFrame({0, 0, 32, 32}, 0.1f);
walk_anim->addFrame({32, 0, 32, 32}, 0.1f);
walk_anim->addFrame({64, 0, 32, 32}, 0.1f);

// 添加并播放
anim->addAnimation(std::move(walk_anim));
anim->playAnimation("walk");
```

### UI 创建

```cpp
// 创建文本
auto text = std::make_unique<UIText>(
    context_,
    "Hello World",
    "assets/fonts/Pixel.ttf",
    24
);
text->setPosition(glm::vec2(100.0f, 50.0f));
ui_manager_->addElement(std::move(text));

// 创建按钮
auto button = std::make_unique<UIButton>(
    context_,
    "assets/textures/button_normal.png",
    "assets/textures/button_hover.png",
    "assets/textures/button_pressed.png",
    glm::vec2(200.0f, 300.0f),
    glm::vec2(150.0f, 50.0f),
    [this]() { this->onButtonClicked(); }
);
ui_manager_->addElement(std::move(button));
```

---

## 事件类型

### EventType (Observer 模式)

```cpp
enum class EventType {
    HEALTH_CHANGED,      // 生命值变化
    MAX_HEALTH_CHANGED,  // 最大生命值变化
    SCORE_CHANGED,       // 分数变化
};
```

---

## 类型定义

### 常用类型

```cpp
// 数学类型
using Vec2 = glm::vec2;
using IVec2 = glm::ivec2;

// 颜色类型
struct FColor {
    float r, g, b, a;
};

// 矩形
struct Rect {
    Vec2 position;
    Vec2 size;
};
```

---

## 更多信息

- 详细架构说明请参考 [01_project_overview.md](01_project_overview.md)
- 核心类图请参考 [02_core_class_diagram.md](02_core_class_diagram.md)
- 关键机制说明请参考 [05_key_mechanisms_part1.md](05_key_mechanisms_part1.md) - [08_key_mechanisms_part4.md](08_key_mechanisms_part4.md)
- 开发规范请参考 [09_development_guidelines.md](09_development_guidelines.md)
- 构建指南请参考 [10_build_guide.md](10_build_guide.md)