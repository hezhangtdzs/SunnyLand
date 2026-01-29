#include "dead_state.h"
#include "../player_component.h"
#include "../../../engine/object/game_object.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/physics_component.h"
//...
void game::component::state::DeadState::enter()
{
    spdlog::debug("玩家进入死亡状态。");
    playAnimation("hurt");  // 播放死亡(受伤)动画

    // 应用击退力（只向上）
    auto physics_component = player_component_->getPhysicsComponent();
    physics_component->velocity_ = glm::vec2(0.0f, -200.0f);  // 向上弹起

    // 禁用碰撞(使其可以掉出屏幕)
    auto collider_component = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>();
    if (collider_component) {
        collider_component->setIsActive(false);
    }
}

void game::component::state::DeadState::exit()
{
}

std::unique_ptr<game::component::state::PlayerState> game::component::state::DeadState::handleInput(engine::core::Context& context)
{
	return std::unique_ptr<PlayerState>();
}

std::unique_ptr<game::component::state::PlayerState> game::component::state::DeadState::update(float delta_time, engine::core::Context& context)
{
	return std::unique_ptr<PlayerState>();
}
