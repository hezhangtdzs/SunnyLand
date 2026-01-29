# 目录结构 (Directory Structure)

```text
src/
├── engine/             # 引擎核心
│   ├── core/           # 基础框架 (App, Context, Time, Config)
│   ├── audio/          # 音频播放封装 (AudioPlayer)
│   ├── scene/          # 场景管理 (Scene, SceneManager, LevelLoader)
│   ├── object/         # 游戏实体 (GameObject)
│   ├── component/      # 组件系统
│   │   ├── transform_component.h
│   │   ├── sprite_component.h
│   │   ├── physics_component.h
│   │   ├── collider_component.h
│   │   ├── animation_component.h
│   │   ├── health_component.h
│   │   ├── audio_component.h
│   │   ├── ai_component.h
│   │   └── behaviors/  # AI 行为策略 (Patrol, UpDown, Jump)
│   ├── physics/         # 物理系统 (PhysicsEngine)
│   ├── render/         # 渲染基础 (Renderer, Camera, Sprite)
│   ├── resource/       # 资源管理 (Texture, Font, Sound)
│   │   └── audio_manager.h
│   ├── input/          # 输入系统 (InputManager)
│   ├── ui/             # UI 系统 (UIManager, UIElement, Button, Text, Panel, Image)
│   │   ├── ui_manager.h
│   │   ├── ui_manager.cpp
│   │   ├── ui_element.h
│   │   ├── ui_element.cpp
│   │   ├── ui_interactive.h
│   │   ├── ui_interactive.cpp
│   │   ├── ui_panel.h
│   │   ├── ui_panel.cpp
│   │   ├── ui_text.h
│   │   ├── ui_text.cpp
│   │   ├── ui_button.h
│   │   ├── ui_button.cpp
│   │   ├── ui_image.h
│   │   ├── ui_image.cpp
│   │   └── state/        # UI 状态管理
│   │       ├── ui_state.h
│   │       ├── normal_state.h
│   │       ├── normal_state.cpp
│   │       ├── hover_state.h
│   │       ├── hover_state.cpp
│   │       ├── pressed_state.h
│   │       └── pressed_state.cpp
│   └── utils/          # 工具类 (Alignment, Math)
└── game/               # 游戏业务逻辑
├── scene/          # 具体场景实现 (GameScene)
│   ├── end_scene.cpp
│   ├── end_scene.h
│   ├── game_scene.cpp
│   ├── game_scene.h
│   ├── helps_scene.cpp
│   ├── helps_scene.h
│   ├── menu_scene.cpp
│   ├── menu_scene.h
│   ├── title_scene.cpp
│   └── title_scene.h
├── component/      # 游戏特定的组件 (PlayerComponent, State Machine)
├── data/           # 游戏数据管理
│   ├── session_data.cpp
│   └── session_data.h
└── sounds/              # 音频资源

docs/                   # 项目文档
├── PROJECT_STRUCTURE.md # 本文档：结构、调用流程与约定
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

