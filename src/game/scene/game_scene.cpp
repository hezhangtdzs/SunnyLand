#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/game_object.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/physics/collider.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/render/camera.h"
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
        engine::scene::LevelLoader level_loader;
        level_loader.loadLevel("assets/maps/level1.tmj", *this);

        auto* main_layer_obj = findGameObjectByName("main");
        if (main_layer_obj) {
            auto* tile_layer_comp = main_layer_obj->getComponent<engine::component::TileLayerComponent>();
            if (tile_layer_comp) {
                context_.getPhysicsEngine().registerCollisionLayer(tile_layer_comp);
				spdlog::trace("已注册主图层的 TileLayerComponent 到物理引擎。");
            }
            // 创建 test_object
            createTestObject();

            Scene::init();
            spdlog::trace("GameScene 初始化完成。");
        }
    }

    void GameScene::update(float delta_time) {
        Scene::update(delta_time);
		TestCollisionPairs();
    }

    void GameScene::render() {
        Scene::render();
    }

    void GameScene::handleInput() {
        Scene::handleInput();
        processTestObjectInput();
    }

    void GameScene::clean() {
        Scene::clean();
    }

    // --- 私有方法 ---

    void GameScene::createTestObject() {
        spdlog::trace("在 GameScene 中创建 test_object...");

        // 物体1: 受重力的箱子 (AABB)
        {
            auto test_object = std::make_unique<engine::object::GameObject>("test_object");
            test_object_ = test_object.get();
            test_object->addComponent<engine::component::TransformComponent>(glm::vec2(100.0f, 100.0f));
            test_object->addComponent<engine::component::SpriteComponent>("assets/textures/Props/big-crate.png", context_.getResourceManager());
            test_object->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine());
            test_object->addComponent<engine::component::ColliderComponent>(
                std::make_unique<engine::physics::AABBCollider>(glm::vec2(32.0f, 32.0f)),
                engine::utils::Alignment::TOP_LEFT,
                false);
            addGameObject(std::move(test_object));
        }
    }

    void GameScene::processTestObjectInput()
    {

        if (!test_object_) return;
        auto& input_manager = context_.getInputManager();
        auto* pc = test_object_->getComponent<engine::component::PhysicsComponent>();
        if (!pc) return;

        // 水平移动: 直接设置速度
        if (input_manager.isActionDown("move_left")) {
            pc->velocity_.x = -100.0f;
        }
        else if (input_manager.isActionDown("move_right")) {
            pc->velocity_.x = 100.0f;
        }
        else {
            // 模拟摩擦力，让物体停下来
            pc->velocity_.x *= 0.9f;
        }

        // 跳跃: 给予一个向上的瞬时速度
        if (input_manager.isActionPressed("jump")) {
            pc->velocity_.y = -400.0f;
        }
    }

	void GameScene::TestCollisionPairs()
	{
		auto& collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
		for (auto& pair : collision_pairs) {
			spdlog::info("碰撞对: {} 和 {}", pair.first->getName(), pair.second->getName());
		}
	}
} // namespace game::scene 