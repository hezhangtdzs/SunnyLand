#include "game_scene.h"
#include "menu_scene.h"
#include "end_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/game_object.h"
#include "../../engine/core/game_state.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/ai_component.h"
#include "../component/behaviors/patrol_behavior.h"
#include "../component/behaviors/up_down_behavior.h"
#include "../component/behaviors/jump_behavior.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/health_component.h"
#include "../../game/component/player_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/audio_component.h"
#include "../../engine/resource/resource_manager.h"
#include "../data/session_data.h"
#include "../object/game_object_builder.h"

#include "../../engine/physics/collider.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/animation.h"
#include "../../engine/render/text_renderer.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_text.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_button.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_rect.h>

namespace game::scene {

    // 构造函数：调用基类构造函数
    GameScene::GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager, std::string level_path)
        : Scene(name, context, scene_manager), level_path_(std::move(level_path)) {
        session_data_ = scene_manager_.getSessionData();
        spdlog::trace("GameScene 构造完成。");
    }

    // 构造函数：带 SessionData 的版本
    GameScene::GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager, std::shared_ptr<game::data::SessionData> session_data, std::string level_path)
        : Scene(name, context, scene_manager), 
          level_path_(std::move(level_path)),
          session_data_(std::move(session_data)) {
        if (!session_data_) {
            session_data_ = scene_manager_.getSessionData();
        }
        spdlog::trace("GameScene 构造完成 (带 SessionData)。");
    }

    void GameScene::init() {
        if (initLevel() && initPlayer() && initEnemyAndItem()) {
            context_.getGameState().setState(engine::core::GameStateType::Playing);
            context_.getResourceManager().playMusic("assets/audio/platformer_level03_loop.ogg");
            spdlog::info("GameScene 初始化完成。");
        }

        Scene::init();
        initHUD(); // 初始化HUD
    }

    void GameScene::update(float delta_time) {
        handleObjectCollisions();
        handleTileTriggers();
        Scene::update(delta_time);
        updateHUD(); // 更新HUD显示

        // 检查游戏失败条件
        if (session_data_) {
            // 检查生命值
            if (session_data_->getCurrentHealth() <= 0) {
                spdlog::info("玩家生命值耗尽，游戏失败！");
                session_data_->setIsWin(false);
                
                // 创建结束场景
                auto end_scene = std::make_unique<EndScene>(
                    context_, 
                    scene_manager_, 
                    session_data_);
                scene_manager_.requestReplaceScene(std::move(end_scene));
                return;
            }
            
            // 检查玩家是否掉出屏幕
            if (player_) {
                auto* transform = player_->getComponent<engine::component::TransformComponent>();
                if (transform) {
                    auto position = transform->getPosition();
                    // 获取相机视口大小
                    auto viewport_size = context_.getCamera().getViewportSize();
                    // 检查玩家是否掉出屏幕下方 100 像素
                    if (position.y > viewport_size.y + 100.0f) {
                        spdlog::info("玩家掉出屏幕，游戏失败！");
                        session_data_->setIsWin(false);
                        
                        // 创建结束场景
                        auto end_scene = std::make_unique<EndScene>(
                            context_, 
                            scene_manager_, 
                            session_data_);
                        scene_manager_.requestReplaceScene(std::move(end_scene));
                        return;
                    }
                }
            }
        }
    }

    void GameScene::render() {
        Scene::render();
    }

    bool GameScene::handleInput() {
        Scene::handleInput();

        if (context_.getInputManager().isActionPressed("pause")) {
            spdlog::debug("在GameScene中检测到暂停动作，正在推送MenuScene。");
            scene_manager_.requestPushScene(std::make_unique<MenuScene>(context_, scene_manager_, session_data_));
        }
        return true;
    }

    void GameScene::clean() {
        Scene::clean();
    }

void GameScene::initHUD() {
    auto* ui_manager = getUIManager();
    if (!ui_manager) {
        return;
    }

    glm::vec2 viewport_size = context_.getCamera().getViewportSize();
    const float padding = 20.0f;
    
    // 创建HUD面板
    auto hud_panel = std::make_unique<engine::ui::UIPanel>(context_);
    hud_panel->setPosition({ 0.0f, 0.0f });
    hud_panel->setSize(viewport_size); 
    hud_panel->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f }); 
    hud_panel->setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f }); 
    hud_panel_ = hud_panel.get();
    
    // 初始化生命值图标 (Sunny Land 风格：左上角)
    if (session_data_) {
        int max_health = session_data_->getMaxHealth();
        for (int i = 0; i < max_health; ++i) {
            glm::vec2 icon_pos = { padding + i * 36.0f, padding }; // 稍微增加边距
            
            // 背景心 (空的容器)
            auto health_bg = std::make_unique<engine::ui::UIImage>(context_, "assets/textures/UI/Heart-bg.png", 
                                                                  icon_pos, 
                                                                  glm::vec2(32.0f, 32.0f));
            hud_panel->addChild(std::move(health_bg));
            
            // 实心红心
            auto health_icon = std::make_unique<engine::ui::UIImage>(context_, "assets/textures/UI/Heart.png", 
                                                                  icon_pos, 
                                                                  glm::vec2(32.0f, 32.0f));
            health_icons_.push_back(health_icon.get());
            hud_panel->addChild(std::move(health_icon));
        }
    }
    
    // 初始化得分显示 (右对齐)
    auto score_text = std::make_unique<engine::ui::UIText>(context_, "Score: 0", "assets/fonts/VonwaonBitmap-16px.ttf", 24);
    score_text->setPosition({ viewport_size.x - padding, padding }); 
    score_text->setAlignment(engine::ui::TextAlignment::RIGHT); // 关键：设置为右对齐
    score_text->setColor({ 1.0f, 1.0f, 1.0f, 1.0f }); 
    
    score_text_ = score_text.get();
    hud_panel->addChild(std::move(score_text));
    
    ui_manager->addElement(std::move(hud_panel));
    updateHUD();
    
}


void GameScene::updateHUD() {
    // 更新得分显示
    if (score_text_ && session_data_) {
        score_text_->setText("Score: " + std::to_string(session_data_->getCurrentScore()));
    }
    
    // 更新生命值图标
    if (session_data_) {
        int current_health = session_data_->getCurrentHealth();
        
        for (size_t i = 0; i < health_icons_.size(); ++i) {
            if (health_icons_[i]) {
                // 根据当前生命值设置图标的可见性
                health_icons_[i]->setVisible(i < current_health);
            }
        }
    }
}

    bool GameScene::initLevel() {
        // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
        engine::scene::LevelLoader level_loader;
        if (!level_loader.loadLevel(level_path_, *this)) {
            spdlog::error("关卡加载失败: {}", level_path_);
            return false;
        }

        // 注册"main"层到物理引擎
        auto* main_layer = findGameObjectByName("main");
        if (!main_layer) {
            spdlog::error("未找到\"main\"层");
            return false;
        }
        auto* tile_layer = main_layer->getComponent<engine::component::TileLayerComponent>();
        if (!tile_layer) {
            spdlog::error("\"main\"层没有 TileLayerComponent 组件");
            return false;
        }
        context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
        spdlog::info("注册\"main\"层到物理引擎");

        // 设置相机边界
        auto world_size = main_layer->getComponent<engine::component::TileLayerComponent>()->getWorldSize();
        context_.getCamera().setLimitBounds(engine::utils::Rect(glm::vec2(0.0f), world_size));

        // 设置世界边界
        context_.getPhysicsEngine().setWorldBounds(engine::utils::Rect(glm::vec2(0.0f), world_size));

        spdlog::trace("关卡初始化完成。");
        return true;
    }

    bool GameScene::initPlayer() {
        player_ = findGameObjectByName("player");
        if (!player_) {
            spdlog::error("未找到玩家对象");
            return false;
        }

        // 添加PlayerComponent到玩家对象
        auto* player_component = player_->addComponent<game::component::PlayerComponent>();
        if (!player_component) {
            spdlog::error("无法添加 PlayerComponent 到玩家对象");
            return false;
        }

    // 如果会话数据存在，初始化玩家生命值
    if (session_data_) {
        auto* health_component = player_->getComponent<engine::component::HealthComponent>();
        if (health_component) {
            // 设置最大生命值和当前生命值
            health_component->setMaxHealth(session_data_->getMaxHealth());
            health_component->setCurrentHealth(session_data_->getCurrentHealth());
            spdlog::info("使用会话数据初始化玩家生命值: {}/{}", 
                session_data_->getCurrentHealth(), session_data_->getMaxHealth());
        }
    }

        // 相机跟随玩家
        auto* player_transform = player_->getComponent<engine::component::TransformComponent>();
        if (!player_transform) {
            spdlog::error("玩家对象没有 TransformComponent 组件, 无法设置相机目标");
            return false;
        }
        context_.getCamera().setTarget(player_transform);

		if (auto* audio = player_->getComponent<engine::component::AudioComponent>()) {
			audio->setMinIntervalMs(80);
			spdlog::trace("玩家音频组件已由关卡数据加载。");
		}

        spdlog::trace("Player初始化完成。");

        return true;
    }

    bool GameScene::initEnemyAndItem() {
        // ============================================================
        // 生成器模式(Builder Pattern) - Director 角色实现
        // ============================================================
        // 角色定义：
        // - Director(指挥者): GameScene::initEnemyAndItem()
        //   职责：控制构建流程，决定构建什么对象、按什么顺序构建
        //
        // - AbstractBuilder(抽象生成器): engine::object::ObjectBuilder
        //   职责：定义构建游戏对象的通用步骤和接口
        //
        // - ConcreteBuilder(具体生成器): game::object::GameObjectBuilder
        //   职责：实现游戏特定对象的构建步骤
        //
        // - Product(产品): engine::object::GameObject
        //   职责：被构建的复杂对象
        // ============================================================

        // 创建 ConcreteBuilder 实例
        // Director 不需要知道具体的构建细节，只需要调用 Builder 的接口
        engine::scene::LevelLoader level_loader;
        game::object::GameObjectBuilder builder(level_loader, context_);

        // Director 控制构建流程：遍历所有游戏对象并增强它们
        for (auto& game_object : game_objects_) {
            const std::string& name = game_object->getName();

            // Director 决定构建什么：
            // 1. 配置类型（通过 autoDetectType）
            // 2. 设置目标对象（通过 enhance）
            // 3. 执行构建（通过 buildEnhancement）
            builder.autoDetectType(name)
                   ->enhance(game_object.get());

            // 执行增强构建
            // ConcreteBuilder 处理具体构建细节：
            // - 如果是敌人，添加 AIComponent 和相应的行为
            // - 如果是玩家，添加 PlayerComponent
            // - 如果是道具，设置标签并播放动画
            if (!builder.buildEnhancement()) {
                spdlog::warn("GameObjectBuilder 未能增强对象 '{}'", name);
            }
        }

        spdlog::info("GameScene::initEnemyAndItem() 完成，共处理 {} 个游戏对象", game_objects_.size());
        return true;
    }

    /**
     * @brief 使用GameObjectBuilder创建游戏对象的示例方法
     *
     * 生成器模式(Builder Pattern)的典型用法：
     * @code
     * // 角色定义：
     * // - Director(指挥者): GameScene，负责调用builder构建对象
     * // - Builder(抽象生成器): engine::object::ObjectBuilder
     * // - ConcreteBuilder(具体生成器): game::object::GameObjectBuilder
     * // - Product(产品): engine::object::GameObject
     *
     * // 使用步骤：
     * // 1. 创建Builder实例
     * // 2. 通过链式调用配置对象属性
     * // 3. 调用build()构建对象
     * // 4. 获取构建好的对象
     * @endcode
     *
     * @note 此方法展示了如何在场景中使用GameObjectBuilder，
     *       实际游戏中可以在LevelLoader中集成使用
     */
    void GameScene::exampleUsageOfGameObjectBuilder() {
        // 当前已在initEnemyAndItem()中实际使用GameObjectBuilder
        // 以下是使用示例的详细说明：

        /*
        // ============================================================
        // 示例1：增强已存在的游戏对象（当前initEnemyAndItem中使用的方式）
        // ============================================================
        // 特点：
        // - 不需要手动调用reset，buildEnhancement()内部会自动清理状态
        // - 使用链式调用配置和构建
        // - 所有游戏特定逻辑（AI、动画等）都封装在Builder中
        
        engine::scene::LevelLoader level_loader;
        game::object::GameObjectBuilder builder(level_loader, context_);

        // 对于每个已存在的游戏对象
        for (auto& game_object : game_objects_) {
            // 链式调用：配置 -> 设置目标 -> 执行增强
            builder.autoDetectType(game_object->getName())      // 自动识别类型
                   .enhance(game_object.get())                  // 设置目标对象
                   .buildEnhancement();                         // 执行增强（内部自动清理状态）
        }

        // ============================================================
        // 示例2：从头创建新对象（可在LevelLoader中使用）
        // ============================================================
        // 特点：
        // - build()内部自动调用reset，无需手动重置
        // - 先构建基础组件，再构建游戏特定组件
        
        game::object::GameObjectBuilder builder(level_loader, context_);
        
        auto eagle = builder
            .configure(&object_json, &tile_json, tile_info)     // 配置基础信息
            .setEnemyType("eagle")                              // 设置敌人类型
            .build()                                            // 构建对象（内部自动reset+构建）
            .getGameObject();                                   // 获取对象

        // ============================================================
        // 示例3：链式调用构建多个对象
        // ============================================================
        // 特点：
        // - 每次build()都会自动重置，可以连续构建不同对象
        
        auto frog = builder
            .configure(&object_json, &tile_json, tile_info)
            .setEnemyType("frog")
            .build()
            .getGameObject();
            
        auto player = builder
            .configure(&object_json, &tile_json, tile_info)
            .setAsPlayer()
            .build()
            .getGameObject();

        // ============================================================
        // 生成器模式的优势：
        // 1. 分离对象的构建与表示：构建过程独立于具体组件
        // 2. 更好的控制构建过程：可以分步骤构建复杂对象
        // 3. 更好的扩展性：添加新类型的对象只需扩展builder
        // 4. 代码复用：builder可以复用于不同场景
        // 5. 自动状态管理：build/buildEnhancement自动处理reset
        // ============================================================
        */
    }

    void GameScene::handleObjectCollisions() {
        // 从物理引擎中获取碰撞对
        auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
        for (const auto& pair : collision_pairs) {
            auto* obj1 = pair.first;
            auto* obj2 = pair.second;

            // 处理关卡切换触发器
            auto checkLevelSwitch = [&](engine::object::GameObject* p, engine::object::GameObject* trigger) {
                if (p->getName() == "player" && (trigger->getTag() == "next_level" || trigger->getName() == "win")) {
                    if (trigger->getName() == "win") {
                        spdlog::info("恭喜！你赢了！");
                        if (session_data_) {
                            session_data_->setIsWin(true);
                        }
                        auto end_scene = std::make_unique<EndScene>(context_, scene_manager_, session_data_);
                        scene_manager_.requestReplaceScene(std::move(end_scene));
                        return true;
                    }

                    std::string next_level_path = "assets/maps/" + trigger->getName() + ".tmj";
                    spdlog::info("玩家触碰关卡切换触发器，准备加载: {}", next_level_path);
                    
                    if (session_data_) {
                        // 准备保存数据
                        session_data_->prepareToSaveData();
                        // 设置新的地图路径
                        session_data_->setMapPath(next_level_path);
                        // 保存游戏状态
                        session_data_->save();
                        // 取消保存数据标志
                        session_data_->cancelSaveData();
                    }

                    auto next_scene = std::make_unique<GameScene>("GameScene", context_, scene_manager_, session_data_, next_level_path);
                    scene_manager_.requestReplaceScene(std::move(next_scene));
                    return true;
                }
                return false;
            };

            if (checkLevelSwitch(obj1, obj2) || checkLevelSwitch(obj2, obj1)) {
                return; // 场景即将替换，跳出循环
            }

            // 处理玩家与敌人的碰撞
            if (obj1->getName() == "player" && obj2->getTag() == "enemy") {
                PlayerVSEnemyCollision(obj1, obj2);
            }
            else if (obj2->getName() == "player" && obj1->getTag() == "enemy") {
                PlayerVSEnemyCollision(obj2, obj1);
            }
            // 处理玩家与道具的碰撞
            else if (obj1->getName() == "player" && obj2->getTag() == "item") {
                PlayerVSItemCollision(obj1, obj2);
            }
            else if (obj2->getName() == "player" && obj1->getTag() == "item") {
                PlayerVSItemCollision(obj2, obj1);
            }
            // 处理玩家与危险物品（如尖刺对象）的碰撞
            else if (obj1->getName() == "player" && obj2->getTag() == "hazard") {
                processHazardDamage(obj1);
            }
            else if (obj2->getName() == "player" && obj1->getTag() == "hazard") {
                processHazardDamage(obj2);
            }
        }
    }

    void GameScene::PlayerVSEnemyCollision(engine::object::GameObject* player, engine::object::GameObject* enemy)
    {
        auto* player_collider = player->getComponent<engine::component::ColliderComponent>();
        auto* enemy_collider = enemy->getComponent<engine::component::ColliderComponent>();
        auto* player_physics = player->getComponent<engine::component::PhysicsComponent>();

        if (!player_collider || !enemy_collider || !player_physics) return;

        auto player_aabb = player_collider->getWorldAABB();
        auto enemy_aabb = enemy_collider->getWorldAABB();

        // 优化判定：使用底部高度和垂直速度
        float player_bottom = player_aabb.position.y + player_aabb.size.y;
        float enemy_middle = enemy_aabb.position.y + enemy_aabb.size.y * 0.5f;

        // 踩踏条件：
        // 1. 玩家正在向下移动 (velocity.y > 0)
        // 2. 玩家底部在敌人中心线上方 (确保是从上方接触)
        bool is_falling = player_physics->velocity_.y > 0.0f;
        bool is_above = player_bottom < enemy_middle;

        // 踩踏判断成功，敌人受伤
        if (is_falling && is_above) {
            spdlog::info("玩家 {} 踩踏了敌人 {}", player->getName(), enemy->getName());
			if (auto* player_audio = player->getComponent<engine::component::AudioComponent>()) {
				player_audio->playSound("stomp", context_);
			}
			if (auto* audio = enemy->getComponent<engine::component::AudioComponent>()) {
				audio->playSoundNearCamera("cry", context_, 420.0f);
			}
            auto enemy_health = enemy->getComponent<engine::component::HealthComponent>();
            if (!enemy_health) {
                // 如果敌人没血条也能被踩死（比如直接移除）
                enemy->setNeedRemove(true);
                
                // 踩踏敌人得分
                const int enemy_score = 50;
                if (session_data_) {
                    session_data_->addScore(enemy_score);
                    spdlog::info("玩家踩踏敌人获得 {} 分，总得分: {}", enemy_score, session_data_->getCurrentScore());
                }
                
                return;
            }

            enemy_health->takeDamage(1);  // 造成1点伤害
            if (!enemy_health->isAlive()) {
                enemy->setNeedRemove(true);  // 标记敌人为待删除状态
                auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;
                createEffect(enemy_center, enemy->getTag());  // 创建（死亡）特效
                
                // 踩踏敌人得分
                const int enemy_score = 50;
                if (session_data_) {
                    session_data_->addScore(enemy_score);
                    spdlog::info("玩家踩踏敌人获得 {} 分，总得分: {}", enemy_score, session_data_->getCurrentScore());
                }
            }
            // 玩家反弹跳起效果
            player_physics->velocity_.y = -300.0f;
        }
        // 踩踏判断失败，玩家受伤
        else {
            processHazardDamage(player);
        }
    }

    void GameScene::PlayerVSItemCollision(engine::object::GameObject* player, engine::object::GameObject* item)
    {
		if (auto* audio = item->getComponent<engine::component::AudioComponent>()) {
			audio->playSound("pickup", context_);
		}
        if (item->getName() == "fruit" || item->getTag() == "fruit") {
            auto* health = player->getComponent<engine::component::HealthComponent>();
            if (health) {
                health->heal(1);  // 加血
                // 更新会话数据中的生命值
                if (session_data_) {
                    session_data_->setCurrentHealth(health->getCurrentHealth());
                }
            }
        }
        else if (item->getName() == "gem" || item->getTag() == "gem") {
            // 加分并更新会话数据
            const int gem_score = 100;
            if (session_data_) {
                session_data_->addScore(gem_score);
                spdlog::info("玩家获得 {} 分，总得分: {}", gem_score, session_data_->getCurrentScore());
            }
        }
        item->setNeedRemove(true);  // 标记道具为待删除状态
        
        // 优先使用碰撞盒中心，如果没有则使用 Transform 位置
        glm::vec2 effect_pos;
        if (auto* cc = item->getComponent<engine::component::ColliderComponent>()) {
            auto aabb = cc->getWorldAABB();
            effect_pos = aabb.position + aabb.size / 2.0f;
        } else if (auto* tc = item->getComponent<engine::component::TransformComponent>()) {
            effect_pos = tc->getPosition(); // 注意：这里可能需要加上精灵尺寸的一半来居中
        } else {
            return;
        }
        
        createEffect(effect_pos, "item");
    }

    void GameScene::handleTileTriggers()
    {
        const auto& tile_trigger_events = context_.getPhysicsEngine().getTileTriggerEvents();
        for (const auto& event : tile_trigger_events) {
            auto* obj = event.first;      // 触发事件的对象
            auto tile_type = event.second;  // 瓦片类型
            
            if (tile_type == engine::component::TileType::HAZARD) {
                // 如果是玩家碰到了危险瓦片，就受伤
                if (obj->getName() == "player") {
                    processHazardDamage(obj);
                }
            }
            else if (tile_type == engine::component::TileType::LEVEL_EXIT) {
                // 如果是玩家碰到了关卡出口，进入下一关
                if (obj->getName() == "player") {
                    spdlog::info("玩家到达关卡出口，准备进入下一关");
                    
                    // 确定下一关的路径
                    std::string next_level_path;
                    if (level_path_ == "assets/maps/level1.tmj") {
                        next_level_path = "assets/maps/level2.tmj";
                    } else if (level_path_ == "assets/maps/level2.tmj") {
                        next_level_path = "assets/maps/level1.tmj"; // 循环回第一关
                    } else {
                        next_level_path = "assets/maps/level1.tmj"; // 默认回第一关
                    }
                    
                    // 保存当前状态（包含更新后的路径）
                    if (session_data_) {
                        // 不再确认分数，保持累积
                        session_data_->setMapPath(next_level_path);
                        // 检查是否需要重置分数
                        session_data_->checkAndResetScore();
                        session_data_->save();
                    }
                    
                    // 请求切换到下一关
                    auto new_scene = std::make_unique<GameScene>(
                        "GameScene", 
                        context_, 
                        scene_manager_, 
                        session_data_,
                        next_level_path);
                    scene_manager_.requestReplaceScene(std::move(new_scene));
                    
                    break; // 只处理第一个出口触发事件
                }
            }
        }
    }

    void GameScene::processHazardDamage(engine::object::GameObject* player)
    {
        auto* player_comp = player->getComponent<game::component::PlayerComponent>();
        if (player_comp) {
            player_comp->takeDamage(1, context_);
            
            if (session_data_) {
                auto* health = player->getComponent<engine::component::HealthComponent>();
                if (health) {
                    session_data_->setCurrentHealth(health->getCurrentHealth());
                    spdlog::info("玩家受伤，生命值更新: {}/{}", 
                        session_data_->getCurrentHealth(), session_data_->getMaxHealth());
                }
            }
        }
    }


    void GameScene::createEffect(const glm::vec2& center_pos, const std::string& tag)
    {
        // 根据标签创建相应的动画
        auto animation = std::make_unique<engine::render::Animation>("effect", false); // false表示不循环
        glm::vec2 offset(0.0f);

        if (tag == "enemy") {
            for (auto i = 0; i < 5; ++i) {
                animation->addFrame({ static_cast<float>(i * 40), 0.0f, 40.0f, 41.0f }, 0.1f);
            }
            offset = glm::vec2(20.0f, 20.5f); // 40x41 center
        }
        else if (tag == "item") {
            for (auto i = 0; i < 4; ++i) {
                animation->addFrame({ static_cast<float>(i * 32), 0.0f, 32.0f, 32.0f }, 0.1f);
            }
            offset = glm::vec2(16.0f, 16.0f); // 32x32 center
        }
        else { return; }

        auto effect_obj = std::make_unique<engine::object::GameObject>("effect_" + tag);
        effect_obj->addComponent<engine::component::TransformComponent>(center_pos - offset);

        if (tag == "enemy") {
            effect_obj->addComponent<engine::component::SpriteComponent>("assets/textures/FX/enemy-deadth.png", context_.getResourceManager());
        }
        else if (tag == "item") {
            effect_obj->addComponent<engine::component::SpriteComponent>("assets/textures/FX/item-feedback.png", context_.getResourceManager());
        }

        // 添加动画组件，并设置为单次播放后自动移除
        auto* animation_component = effect_obj->addComponent<engine::component::AnimationComponent>();
        animation_component->addAnimation(std::move(animation));
        animation_component->setOneShotRemoval(true);
        animation_component->playAnimation("effect");

        safeAddGameObject(std::move(effect_obj));  // 安全添加特效对象
    }


    
}