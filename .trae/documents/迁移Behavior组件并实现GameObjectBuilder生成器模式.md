## 任务概述

完成两个主要任务：
1. 将 `d:\repos\Game\SunnyLand\src\game\component\behaviors` 下的组件从 `engine::component` 命名空间迁移至 `game::component` 命名空间
2. 在 `d:\repos\Game\SunnyLand\src\game\scene` 目录下实现生成器模式，创建继承自 `engine::object::ObjectBuilder` 的 `game::object::GameObjectBuilder` 类

---

## 第一阶段：命名空间迁移

### 1.1 修改Behavior头文件命名空间

**文件列表：**
- `d:\repos\Game\SunnyLand\src\game\component\behaviors\jump_behavior.h`
- `d:\repos\Game\SunnyLand\src\game\component\behaviors\patrol_behavior.h`
- `d:\repos\Game\SunnyLand\src\game\component\behaviors\up_down_behavior.h`

**修改内容：**
- 将 `namespace engine::component` 改为 `namespace game::component`
- 保持继承关系 `engine::component::AIBehavior` 不变（基类仍在engine命名空间）

### 1.2 修改Behavior实现文件命名空间

**文件列表：**
- `d:\repos\Game\SunnyLand\src\game\component\behaviors\jump_behavior.cpp`
- `d:\repos\Game\SunnyLand\src\game\component\behaviors\patrol_behavior.cpp`
- `d:\repos\Game\SunnyLand\src\game\component\behaviors\up_down_behavior.cpp`

**修改内容：**
- 将 `engine::component::JumpBehavior` 等改为 `game::component::JumpBehavior`
- 更新所有方法定义的命名空间前缀

### 1.3 更新所有引用点

**需要修改的文件：**
- `d:\repos\Game\SunnyLand\src\game\scene\game_scene.cpp` (第11-13行)
  - 修改 `#include` 路径（如有需要）
  - 修改使用处的命名空间：`engine::component::UpDownBehavior` → `game::component::UpDownBehavior`
  - 同理修改 `JumpBehavior` 和 `PatrolBehavior`

---

## 第二阶段：实现GameObjectBuilder生成器模式

### 2.1 创建GameObjectBuilder头文件

**新文件：** `d:\repos\Game\SunnyLand\src\game\object\game_object_builder.h`

**设计要点：**
```cpp
namespace game::object {
    class GameObjectBuilder : public engine::object::ObjectBuilder {
    public:
        GameObjectBuilder(engine::scene::LevelLoader& level_loader, 
                         engine::core::Context& context);
        
        // 重写build方法，添加游戏特定组件
        void build() override;
        
        // 添加游戏特定构建步骤
        void buildAI();        // 添加AI行为组件
        void buildPlayer();    // 添加玩家特定组件
        void buildEnemy();     // 添加敌人特定组件
        void buildItem();      // 添加道具特定组件
        
        // 配置接口
        GameObjectBuilder* setBehaviorType(const std::string& type);
        GameObjectBuilder* setEnemyType(const std::string& enemy_type);
    };
}
```

### 2.2 创建GameObjectBuilder实现文件

**新文件：** `d:\repos\Game\SunnyLand\src\game\object\game_object_builder.cpp`

**实现要点：**
- 调用父类的 `build()` 方法完成基础构建
- 根据对象名称/标签添加特定的游戏组件：
  - "eagle" → `UpDownBehavior`
  - "frog" → `JumpBehavior`
  - "opossum" → `PatrolBehavior`
  - "player" → `PlayerComponent`
  - "item"/"fruit"/"gem" → 设置动画和标签

### 2.3 创建示例使用代码

**新文件：** `d:\repos\Game\SunnyLand\src\game\object\game_object_builder_example.cpp` (可选，用于演示)

---

## 第三阶段：更新LevelLoader以支持GameObjectBuilder

### 3.1 修改LevelLoader

**文件：** `d:\repos\Game\SunnyLand\src\engine\scene\level_loader.cpp`

**修改方案（二选一）：**

**方案A - 模板化（推荐）：**
```cpp
template<typename BuilderType = engine::object::ObjectBuilder>
void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene) {
    BuilderType builder(*this, scene.getContext());
    // ... 使用builder构建对象
}
```

**方案B - 依赖注入：**
- 在 `LevelLoader` 中添加设置自定义builder的接口
- 允许外部传入自定义的builder工厂

---

## 第四阶段：单元测试

### 4.1 创建测试文件

**新文件：** `d:\repos\Game\SunnyLand\tests\game_object_builder_test.cpp`

**测试用例：**
1. **基础构建测试**：验证GameObjectBuilder能正确构建基础对象
2. **AI组件测试**：验证不同敌人类型能正确添加对应的AIBehavior
3. **玩家组件测试**：验证player对象能正确添加PlayerComponent
4. **道具组件测试**：验证item对象能正确设置动画和标签
5. **继承关系测试**：验证GameObjectBuilder正确继承ObjectBuilder的功能

---

## 文件修改清单

### 修改现有文件（6个）：
1. `src/game/component/behaviors/jump_behavior.h`
2. `src/game/component/behaviors/jump_behavior.cpp`
3. `src/game/component/behaviors/patrol_behavior.h`
4. `src/game/component/behaviors/patrol_behavior.cpp`
5. `src/game/component/behaviors/up_down_behavior.h`
6. `src/game/component/behaviors/up_down_behavior.cpp`
7. `src/game/scene/game_scene.cpp`

### 新建文件（3个）：
1. `src/game/object/game_object_builder.h`
2. `src/game/object/game_object_builder.cpp`
3. `tests/game_object_builder_test.cpp` (可选)

---

## 预期结果

1. **命名空间迁移完成**：所有Behavior组件位于 `game::component` 命名空间
2. **生成器模式实现**：`GameObjectBuilder` 继承 `ObjectBuilder`，支持游戏特定组件的构建
3. **代码结构优化**：游戏逻辑与引擎核心分离，提高可维护性
4. **功能完整性**：所有原有功能保持不变，新增builder可正常工作

---

## 风险与注意事项

1. **基类AIBehavior位置**：`AIBehavior` 基类仍在 `engine::component`，派生类需要在 `game::component` 中正确引用
2. **前向声明**：确保所有前向声明使用正确的命名空间
3. **CMake配置**：新建文件需要添加到CMakeLists.txt
4. **循环依赖**：避免game和engine模块之间的循环依赖