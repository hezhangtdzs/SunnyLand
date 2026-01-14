# 项目结构与调用流程 (Project Structure and Call Flow)

## 核心类图 (Core Class Diagram)

```mermaid
classDiagram
    class GameApp {
        -is_running_ : bool
        -window_ : SDL_Window*
        -renderer_ : unique_ptr<Renderer>
        -resource_manager_ : unique_ptr<ResourceManager>
        -input_manager_ : unique_ptr<InputManager>
        -time_ : unique_ptr<Time>
        +run()
        +init()
        -handleEvents()
        -update(float delta_time)
        -render()
    }

    class GameObject {
        -name_ : string
        -components_ : map<type_index, unique_ptr<Component>>
        +addComponent<T>()
        +getComponent<T>()
        +update(float delta_time, Context& context)
        +render(Context& context)
    }

    class Component {
        <<interface>>
        #owner_ : GameObject*
        +init()*
        +update(float dt, Context& ctx)*
        +render(Context& ctx)*
    }

    class TransformComponent {
        -position_ : vec2
        -rotation_ : float
        -scale_ : vec2
        +setPosition(vec2)
        +getScale()
    }

    class SpriteComponent {
        -sprite_ : Sprite
        -offset_ : vec2
        -alignment_ : Alignment
        +init()
        +updateOffset()
        +render(Context& ctx)
    }

    class Context {
        -renderer_ : Renderer&
        -camera_ : Camera&
        -resource_manager_ : ResourceManager&
        -input_manager_ : InputManager&
    }

    class Renderer {
        +drawSprite(Camera&, Sprite&, ...)
        +clearScreen()
        +present()
    }

    class ResourceManager {
        +getTexture(string)
        +getFont(string)
    }

    Component <|-- TransformComponent : 继承 (Inherit)
    Component <|-- SpriteComponent : 继承 (Inherit)
    GameObject "1" *-- "many" Component : 包含 (Contains)
    GameApp "1" *-- "many" GameObject : 逻辑持有
    GameApp "1" *-- "1" Renderer : 拥有
    GameApp "1" *-- "1" ResourceManager : 拥有
    GameApp "1" *-- "1" InputManager : 拥有
    GameApp "1" *-- "1" Time : 拥有
    SpriteComponent ..> TransformComponent : 依赖位置与缩放
    SpriteComponent ..> Context : 使用系统资源访问
```

## 主循环调用流程 (Main Loop Sequence)

```mermaid
sequenceDiagram
    participant Main as main.cpp
    participant App as GameApp
    participant Time as Time
    participant Input as InputManager
    participant GO as GameObject
    participant Comp as Component
    participant Scene as Renderer/Camera

    Main->>App: run()
    App->>App: init() (初始化SDL, 渲染器, 资源等)
    
    loop 游戏主循环 (while is_running_)
        App->>Time: update() (计算 delta_time)
        App->>Input: Update() (处理键盘/鼠标状态)
        App->>App: handleEvents() (处理退出等事件)
        
        App->>App: update(delta_time)
        Note right of App: 逻辑更新阶段
        App->>GO: update(delta_time, context)
        GO->>Comp: update(delta_time, context)
        
        App->>App: render()
        Note right of App: 渲染阶段
        App->>Scene: clearScreen()
        App->>GO: render(context)
        GO->>Comp: render(context)
        Comp->>Scene: drawSprite(...)
        App->>Scene: present() (显示图像)
    end
    
    App->>App: close() (清理资源)
```

## 关键系统说明 (Key System Descriptions)

| 系统名称 | 职责描述 | 核心函数 |
| :--- | :--- | :--- |
| **GameApp** | 引擎入口，管理生命周期。 | `run()`, `init()`, `close()` |
| **GameObject** | 游戏实体，通过组合不同的组件来实现特定功能。 | `addComponent()`, `update()`, `render()` |
| **Component** | 组件基类，定义了游戏逻辑和渲染的统一接口。 | `init()`, `update()`, `render()` |
| **TransformComponent** | **最基础组件**，管理对象的位置、旋转、缩放。 | `getPosition()`, `setScale()` |
| **SpriteComponent** | 渲染组件，负责根据 Transform 的信息绘制图片。 | `render()`, `updateOffset()` |
| **Renderer** | 绘图核心，封装对 SDL 渲染 API 的底层调用。 | `drawSprite()`, `clearScreen()` |
| **ResourceManager**| 资源管家，负责图片、字体、声音的加载与缓存。 | `getTexture()`, `getFont()` |
| **InputManager** | 输入核心，将按键/点击映射为抽象的游戏动作。 | `Update()`, `isActionPressed()` |
| **Context** | 系统容器，将各种全局系统引用传递给组件层。 | - |

---

## 精灵与变换的关系 (Relation: Sprite & Transform)

1.  **位置绑定**: `SpriteComponent::render` 执行时，会从所属的 `GameObject` 中获取 `TransformComponent` 的位置。
2.  **坐标转换**: 最终屏幕坐标 = `Transform.Position + Sprite.Offset`。
3.  **自动缩放**: `Transform` 的 `scale` 发生变化时，`SpriteComponent` 会自动重新计算 `offset` 以保持对齐。
4.  **初始化**: `SpriteComponent::init` 时会缓存 `Transform` 的指针，确保高效访问。


