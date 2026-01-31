#include "game_object_builder.h"
#include "../../engine/object/game_object.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/core/context.h"
#include "../../engine/component/ai_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../component/behaviors/up_down_behavior.h"
#include "../component/behaviors/jump_behavior.h"
#include "../component/behaviors/patrol_behavior.h"
#include "../component/player_component.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace game::object {

    GameObjectBuilder::GameObjectBuilder(engine::scene::LevelLoader& level_loader,
                                         engine::core::Context& context)
        : engine::object::ObjectBuilder(level_loader, context),
          target_object_(nullptr) {
        spdlog::trace("GameObjectBuilder 构造成功");
    }

    void GameObjectBuilder::build() {
        // 1. 重置状态（确保每次构建都是干净的）
        resetBuilder();

        // 2. 调用父类build()构建基础组件
        engine::object::ObjectBuilder::build();

        // 3. 构建游戏特定组件
        auto* game_object = getGameObject().get();
        if (game_object) {
            buildGameSpecific(game_object);
        }
    }

    GameObjectBuilder* GameObjectBuilder::enhance(engine::object::GameObject* game_object) {
        target_object_ = game_object;
        return this;
    }

    bool GameObjectBuilder::buildEnhancement() {
        if (!target_object_) {
            spdlog::warn("GameObjectBuilder::buildEnhancement() - target_object为空");
            return false;
        }

        // 根据配置为目标对象添加游戏特定组件
        // 注意：配置（enemy_type_, item_type_, is_player_）应该在此之前通过set方法设置好
        buildGameSpecific(target_object_);

        // 构建完成后重置状态，为下一次构建做准备
        resetGameBuilder();
        target_object_ = nullptr;

        return true;
    }

    GameObjectBuilder* GameObjectBuilder::setEnemyType(const std::string& type) {
        enemy_type_ = type;
        return this;
    }

    GameObjectBuilder* GameObjectBuilder::setItemType(const std::string& type) {
        item_type_ = type;
        return this;
    }

    GameObjectBuilder* GameObjectBuilder::setAsPlayer() {
        is_player_ = true;
        return this;
    }

    GameObjectBuilder* GameObjectBuilder::autoDetectType(const std::string& name) {
        // 根据名称自动推断对象类型
        if (name == "eagle") {
            enemy_type_ = "eagle";
        } else if (name == "frog") {
            enemy_type_ = "frog";
        } else if (name == "opossum") {
            enemy_type_ = "opossum";
        } else if (name == "player") {
            is_player_ = true;
        } else if (name == "fruit") {
            item_type_ = "fruit";
        } else if (name == "gem") {
            item_type_ = "gem";
        }

        spdlog::debug("GameObjectBuilder 自动推断类型: name={}, enemy={}, player={}, item={}",
                      name, enemy_type_.value_or("none"), is_player_, item_type_.value_or("none"));

        return this;
    }

    GameObjectBuilder* GameObjectBuilder::resetBuilder() {
        // 重置父类状态
        reset();
        // 重置本类状态
        resetGameBuilder();
        target_object_ = nullptr;
        return this;
    }

    void GameObjectBuilder::buildGameSpecific(engine::object::GameObject* game_object) {
        if (!game_object) {
            spdlog::warn("GameObjectBuilder::buildGameSpecific() - game_object为空");
            return;
        }

        // 根据配置构建相应组件
        if (enemy_type_.has_value()) {
            buildEnemyAI(game_object);
        }

        if (is_player_) {
            buildPlayerComponent(game_object);
        }

        if (item_type_.has_value()) {
            buildItemComponents(game_object);
        }
    }

    void GameObjectBuilder::buildEnemyAI(engine::object::GameObject* game_object) {
        if (!game_object || !enemy_type_.has_value()) {
            return;
        }

        const std::string& type = enemy_type_.value();
        std::unique_ptr<engine::component::AIBehavior> behavior = nullptr;

        if (type == "eagle") {
            // Eagle: 上下飞行行为
            behavior = std::make_unique<game::component::UpDownBehavior>(40.0f, 80.0f);
            spdlog::info("GameObjectBuilder: 为 '{}' 添加 UpDownBehavior", game_object->getName());
        }
        else if (type == "frog") {
            // Frog: 跳跃行为
            auto* transform = game_object->getComponent<engine::component::TransformComponent>();
            float start_x = transform ? transform->getPosition().x : 0.0f;
            float x_max = start_x - 10.0f;
            float x_min = x_max - 90.0f;

            behavior = std::make_unique<game::component::JumpBehavior>(x_min, x_max, 60.0f, 250.0f, 2.0f);
            spdlog::info("GameObjectBuilder: 为 '{}' 添加 JumpBehavior, 范围: [{}, {}]",
                        game_object->getName(), x_min, x_max);
        }
        else if (type == "opossum") {
            // Opossum: 巡逻行为
            behavior = std::make_unique<game::component::PatrolBehavior>(50.0f, 200.0f);
            spdlog::info("GameObjectBuilder: 为 '{}' 添加 PatrolBehavior", game_object->getName());
        }

        if (behavior) {
            game_object->addComponent<engine::component::AIComponent>(std::move(behavior));
            // 设置敌人标签
            game_object->setTag("enemy");
        }
    }

    void GameObjectBuilder::buildPlayerComponent(engine::object::GameObject* game_object) {
        if (!game_object) {
            return;
        }

        // 添加PlayerComponent
        auto* player_comp = game_object->addComponent<game::component::PlayerComponent>();
        if (player_comp) {
            spdlog::info("GameObjectBuilder: 为 '{}' 添加 PlayerComponent", game_object->getName());
        }

        // 设置玩家标签
        game_object->setTag("player");
    }

    void GameObjectBuilder::buildItemComponents(engine::object::GameObject* game_object) {
        if (!game_object || !item_type_.has_value()) {
            return;
        }

        const std::string& type = item_type_.value();

        // 设置道具标签
        game_object->setTag("item");

        // 播放idle动画
        if (auto* anim = game_object->getComponent<engine::component::AnimationComponent>()) {
            anim->playAnimation("idle");
            spdlog::info("GameObjectBuilder: 为 '{}' 播放 idle 动画", game_object->getName());
        }

        spdlog::info("GameObjectBuilder: 构建 '{}' 道具完成, 类型: {}",
                    game_object->getName(), type);
    }

    void GameObjectBuilder::resetGameBuilder() {
        enemy_type_.reset();
        item_type_.reset();
        is_player_ = false;
    }

} // namespace game::object
