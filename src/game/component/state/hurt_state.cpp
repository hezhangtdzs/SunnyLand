#include "hurt_state.h"
#include "idle_state.h"
#include "fall_state.h"
#include "../player_component.h"
#include <glm/glm.hpp>
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/core/context.h"

void game::component::state::HurtState::enter()
{
    playAnimation("hurt");  // 播放受伤动画
    // --- 造成击退效果 ---
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();
    auto knockback_velocity = glm::vec2(-100.0f, -150.0f); // 默认左上方击退
    // 根据当前精灵的朝向状态决定是否改成右上方
    if (sprite_component->isFlipped()) {
        knockback_velocity.x = -knockback_velocity.x;  // 变成向右
    }
    physics_component->velocity_ = knockback_velocity;
}

void game::component::state::HurtState::exit()
{
}

std::unique_ptr<game::component::state::PlayerState> game::component::state::HurtState::handleInput(engine::core::Context& context)
{
    return nullptr;
}

std::unique_ptr<game::component::state::PlayerState> game::component::state::HurtState::update(float delta_time, engine::core::Context& context)
{
	stunned_timer_ += delta_time;
	auto physics_component = player_component_->getPhysicsComponent();

    if (physics_component->hasCollidedBelow()) {
		return std::make_unique<IdleState>(player_component_);
    }
    else if (physics_component->hasCollidedAbove()) {
		return std::make_unique<FallState>(player_component_);
    }
	return nullptr;
}
