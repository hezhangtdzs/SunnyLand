# 目录结构 (Directory Structure)

```text
src/
├── engine/             # 引擎核心
│   ├── core/           # 基础框架 (App, Context, Time, Config, GameState)
│   │   ├── game_app.h/cpp
│   │   ├── context.h/cpp
│   │   ├── time.h/cpp
│   │   ├── config.h/cpp
│   │   └── game_state.h/cpp
│   ├── audio/          # 音频播放封装 (AudioPlayer)
│   │   ├── audio_player.h/cpp
│   ├── scene/          # 场景管理 (Scene, SceneManager, LevelLoader)
│   │   ├── scene.h/cpp
│   │   ├── scene_manager.h/cpp
│   │   ├── level_loader.h/cpp
│   ├── object/         # 游戏实体 (GameObject, ObjectBuilder)
│   │   ├── game_object.h/cpp
│   │   ├── object_builder.h/cpp
│   ├── component/      # 组件系统
│   │   ├── component.h
│   │   ├── transform_component.h/cpp
│   │   ├── sprite_component.h/cpp
│   │   ├── parallax_component.h/cpp
│   │   ├── physics_component.h/cpp
│   │   ├── collider_component.h/cpp
│   │   ├── animation_component.h/cpp
│   │   ├── health_component.h/cpp
│   │   ├── audio_component.h/cpp
│   │   ├── ai_component.h/cpp
│   │   ├── ai_behavior.h
│   │   └── tilelayer_component.h/cpp
│   ├── physics/        # 物理系统 (PhysicsEngine, Collision)
│   │   ├── physics_engine.h/cpp
│   │   ├── collision.h/cpp
│   │   └── collider.h
│   ├── render/         # 渲染基础 (Renderer, Camera, Sprite, Animation, TextRenderer)
│   │   ├── renderer.h/cpp
│   │   ├── camera.h/cpp
│   │   ├── sprite.h
│   │   ├── animation.h/cpp
│   │   └── text_renderer.h/cpp
│   ├── resource/       # 资源管理 (ResourceManager, TextureManager, FontManager, AudioManager)
│   │   ├── resource_manager.h/cpp
│   │   ├── texture_manager.h/cpp
│   │   ├── font_manager.h/cpp
│   │   └── audio_manager.h/cpp
│   ├── input/          # 输入系统 (InputManager)
│   │   └── input_manager.h/cpp
│   ├── ui/             # UI 系统 (UIManager, UIElement, Button, Text, Panel, Image)
│   │   ├── ui_manager.h/cpp
│   │   ├── ui_element.h/cpp
│   │   ├── ui_interactive.h/cpp
│   │   ├── ui_panel.h/cpp
│   │   ├── ui_text.h/cpp
│   │   ├── ui_button.h/cpp
│   │   ├── ui_image.h/cpp
│   │   └── state/        # UI 状态管理
│   │       ├── ui_state.h
│   │       ├── normal_state.h/cpp
│   │       ├── hover_state.h/cpp
│   │       └── pressed_state.h/cpp
│   └── utils/          # 工具类 (Alignment, Math)
│       ├── alignment.h
│       └── math.h
└── game/               # 游戏业务逻辑
    ├── scene/          # 具体场景实现
    │   ├── game_scene.h/cpp
    │   ├── title_scene.h/cpp
    │   ├── menu_scene.h/cpp
    │   ├── helps_scene.h/cpp
    │   └── end_scene.h/cpp
    ├── component/      # 游戏特定的组件
    │   ├── player_component.h/cpp
    │   ├── state/        # 玩家状态机
    │   │   ├── player_state.h/cpp
    │   │   ├── idle_state.h/cpp
    │   │   ├── walk_state.h/cpp
    │   │   ├── jump_state.h/cpp
    │   │   ├── fall_state.h/cpp
    │   │   ├── hurt_state.h/cpp
    │   │   ├── dead_state.h/cpp
    │   │   └── climb_state.h/cpp
    │   └── behaviors/    # AI 行为策略
    │       ├── patrol_behavior.h/cpp
    │       ├── up_down_behavior.h/cpp
    │       └── jump_behavior.h/cpp
    ├── object/         # 游戏对象生成器
    │   └── game_object_builder.h/cpp
    └── data/           # 游戏数据管理
        ├── session_data.h/cpp

docs/                   # 项目文档
└── architecture/        # 架构文档
    ├── 01_project_overview.md
    ├── 02_core_class_diagram.md
    ├── 03_main_loop_sequence.md
    ├── 04_directory_structure.md
    ├── 05_key_mechanisms_part1.md
    ├── 06_key_mechanisms_part2.md
    ├── 07_key_mechanisms_part3.md
    ├── 08_key_mechanisms_part4.md
    └── 09_development_guidelines.md

