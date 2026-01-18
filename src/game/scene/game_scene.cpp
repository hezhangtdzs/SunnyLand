#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/game_object.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/collider_component.h"
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

        // 创建 test_object
        createTestObject();

        Scene::init();
        spdlog::trace("GameScene 初始化完成。");
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
                engine::utils::Alignment::CENTER);
            addGameObject(std::move(test_object));
        }

        // 物体2: 静止的箱子 (Circle)
        {
            auto test_object2 = std::make_unique<engine::object::GameObject>("test_object2");
            test_object2->addComponent<engine::component::TransformComponent>(glm::vec2(50.0f, 250.0f)); // 放在下落路径上
            test_object2->addComponent<engine::component::SpriteComponent>("assets/textures/Props/big-crate.png", context_.getResourceManager());
            test_object2->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine(), false); // 不受重力
            test_object2->addComponent<engine::component::ColliderComponent>(
                std::make_unique<engine::physics::CircleCollider>(16.0f),
                engine::utils::Alignment::CENTER);
            addGameObject(std::move(test_object2));
        }

        spdlog::trace("test_object/test_object2 创建并添加到 GameScene 中。");
    }

    void GameScene::processTestObjectInput()
    {
        if (!test_object_) return;
        auto& input_manager = context_.getInputManager();
        auto* physics_comp = test_object_->getComponent<engine::component::PhysicsComponent>();
        if (!physics_comp) return;

        // 左右移动暂时还是直接改变位置
        if (input_manager.isActionDown("move_left")) {
            test_object_->getComponent<engine::component::TransformComponent>()->translate(glm::vec2(-2, 0));
        }
        if (input_manager.isActionDown("move_right")) {
            test_object_->getComponent<engine::component::TransformComponent>()->translate(glm::vec2(2, 0));
        }
        if (input_manager.isActionDown("move_up")) {
            test_object_->getComponent<engine::component::TransformComponent>()->translate(glm::vec2(0, -2));
        }
        if (input_manager.isActionDown("move_down")) {
            test_object_->getComponent<engine::component::TransformComponent>()->translate(glm::vec2(0, 2));
        }
        
        // 按下跳跃键时，给予一个向上的瞬时速度 (如果启用了重力)
        if (input_manager.isActionPressed("jump")) {
            physics_comp->setVelocity(glm::vec2(physics_comp->getVelocity().x, -400));
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