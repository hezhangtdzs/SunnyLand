#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/game_object.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/ai_component.h"
#include "../../engine/component/behaviors/patrol_behavior.h"
#include "../../engine/component/behaviors/up_down_behavior.h"
#include "../../engine/component/behaviors/jump_behavior.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/health_component.h"
#include "../../game/component/player_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/physics/collider.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/animation.h"
#include "../../engine/physics/physics_engine.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_rect.h>

namespace game::scene {

    // 构造函数：调用基类构造函数
    GameScene::GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
        : Scene(name, context, scene_manager) {
        spdlog::trace("GameScene 构造完成。");
    }

    void GameScene::init() {
        if (initLevel() && initPlayer() && initEnemyAndItem()) {
            spdlog::info("GameScene 初始化完成。");
        }
        Scene::init();
    }

    void GameScene::update(float delta_time) {
        handleObjectCollisions();
        handleTileTriggers();
        Scene::update(delta_time);
    }

    void GameScene::render() {
        Scene::render();
    }

    void GameScene::handleInput() {
        Scene::handleInput();
    }

    void GameScene::clean() {
        Scene::clean();
    }

    bool GameScene::initLevel()
    {
        // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
        engine::scene::LevelLoader level_loader;
        if (!level_loader.loadLevel("assets/maps/level1.tmj", *this)) {
            spdlog::error("关卡加载失败");
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

    bool GameScene::initPlayer()
    {
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

        // 相机跟随玩家
        auto* player_transform = player_->getComponent<engine::component::TransformComponent>();
        if (!player_transform) {
            spdlog::error("玩家对象没有 TransformComponent 组件, 无法设置相机目标");
            return false;
        }
        context_.getCamera().setTarget(player_transform);
        spdlog::trace("Player初始化完成。");
        return true;
    }
    bool GameScene::initEnemyAndItem()
    {
         bool success = true;
    for (auto& game_object : game_objects_){
        if (game_object->getName() == "eagle"){
            if (auto* ai_component = game_object->addComponent<engine::component::AIComponent>(
                std::make_unique<engine::component::UpDownBehavior>(40.0f, 80.0f)); ai_component){
                spdlog::info("为Eagle添加了UpDownBehavior");
                /*if (auto* ac = game_object->getComponent<engine::component::AnimationComponent>(); ac) {
                    ac->playAnimation("fly");
                }*/
            }
        }
        if (game_object->getName() == "frog"){
            auto* transform = game_object->getComponent<engine::component::TransformComponent>();
            float start_x = transform->getPosition().x;
            float x_max = start_x - 10.0f;
            float x_min = x_max - 90.0f;

            if (auto* ai_component = game_object->addComponent<engine::component::AIComponent>(
                std::make_unique<engine::component::JumpBehavior>(x_min, x_max, 60.0f, 250.0f, 2.0f)); ai_component){
                spdlog::info("为Frog添加了JumpBehavior, 范围: [{}, {}]", x_min, x_max);
                
               /* if (auto* ac = game_object->getComponent<engine::component::AnimationComponent>(); ac) {
                    ac->playAnimation("idle");
                }*/
            }
        }
        if (game_object->getName() == "opossum"){
            if (auto* ai_component = game_object->addComponent<engine::component::AIComponent>(
                std::make_unique<engine::component::PatrolBehavior>(50.0f, 200.0f)); ai_component){
                spdlog::info("为Opossum添加了PatrolBehavior");
              /*  if (auto* ac = game_object->getComponent<engine::component::AnimationComponent>(); ac) {
                    ac->playAnimation("walk");
                }*/
            }
        }
            if (game_object->getTag() == "item") {
                if (auto* ac = game_object->getComponent<engine::component::AnimationComponent>(); ac) {
                    ac->playAnimation("idle");
                }
                else {
                    spdlog::error("Item对象缺少 AnimationComponent，无法播放动画。");
                    success = false;
                }
            }
        }
        return success;
    }
    void GameScene::handleObjectCollisions()
    {
        // 从物理引擎中获取碰撞对
        auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
        for (const auto& pair : collision_pairs) {
            auto* obj1 = pair.first;
            auto* obj2 = pair.second;

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
        float enemy_top = enemy_aabb.position.y;
        float enemy_middle = enemy_aabb.position.y + enemy_aabb.size.y * 0.5f;

        // 踩踏条件：
        // 1. 玩家正在向下移动 (velocity.y > 0)
        // 2. 玩家底部在敌人中心线上方 (确保是从上方接触)
        bool is_falling = player_physics->velocity_.y > 0.0f;
        bool is_above = player_bottom < enemy_middle;

        // 踩踏判断成功，敌人受伤
        if (is_falling && is_above) {
            spdlog::info("玩家 {} 踩踏了敌人 {}", player->getName(), enemy->getName());
            auto enemy_health = enemy->getComponent<engine::component::HealthComponent>();
            if (!enemy_health) {
                // 如果敌人没血条也能被踩死（比如直接移除）
                enemy->setNeedRemove(true);
                return;
            }

            enemy_health->takeDamage(1);  // 造成1点伤害
            if (!enemy_health->isAlive()) {
                enemy->setNeedRemove(true);  // 标记敌人为待删除状态
                auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;
                createEffect(enemy_center, enemy->getTag());  // 创建（死亡）特效
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
        if (item->getName() == "fruit" || item->getTag() == "fruit") {
            auto* health = player->getComponent<engine::component::HealthComponent>();
            if (health) health->heal(1);  // 加血
        }
        else if (item->getName() == "gem" || item->getTag() == "gem") {
            //TODO: 加分
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
        }
    }

    void GameScene::processHazardDamage(engine::object::GameObject* player)
    {
        auto* player_comp = player->getComponent<game::component::PlayerComponent>();
        if (player_comp) {
            player_comp->takeDamage(1);
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

   