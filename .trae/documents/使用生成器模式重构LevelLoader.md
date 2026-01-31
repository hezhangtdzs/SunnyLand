## 目标
使用生成器模式重构LevelLoader，将复杂的对象构建逻辑从LevelLoader中剥离到ObjectBuilder中。

## 具体步骤

### 第一步：完善 ObjectBuilder 实现
创建 `object_builder.cpp`，实现以下方法：
1. 构造函数和析构函数
2. `configure()` 方法（两个重载版本）
3. `build()` 方法 - 按顺序调用各个组件构建方法
4. `getGameObject()` 方法 - 返回构建好的GameObject
5. `reset()` 方法 - 重置生成器状态
6. 各个组件构建方法：
   - `buildBase()` - 构建基础GameObject
   - `buildTransform()` - 构建TransformComponent
   - `buildSprite()` - 构建SpriteComponent
   - `buildPhysics()` - 构建PhysicsComponent和ColliderComponent
   - `buildAnimation()` - 构建AnimationComponent
   - `buildAudio()` - 构建AudioComponent
   - `buildHealth()` - 构建HealthComponent
7. 代理方法（调用LevelLoader的私有方法）：
   - `getTileProperty()`
   - `getTileType()`
   - `getCollisionRect()`
   - `addAnimationFromTileJson()`
   - `addSound()`

### 第二步：重构 LevelLoader
修改 `level_loader.cpp` 中的 `loadObjectLayer` 方法：
1. 创建ObjectBuilder实例
2. 使用builder->configure()配置生成器
3. 调用builder->build()构建对象
4. 使用builder->getGameObject()获取构建好的对象
5. 移除原先直接创建GameObject的逻辑

### 第三步：验证
1. 确保所有组件都能正确创建
2. 确保所有属性都能正确解析
3. 确保代码能正常编译运行

## 预期效果
- LevelLoader职责单一化，只负责解析地图数据
- ObjectBuilder负责具体的对象构建逻辑
- 代码更易维护和扩展
- 新增组件只需修改ObjectBuilder