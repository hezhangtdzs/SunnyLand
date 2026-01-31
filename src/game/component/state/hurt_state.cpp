#include "hurt_state.h"
#include "idle_state.h"
#include "fall_state.h"
#include "../player_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/core/context.h"
#include <glm/glm.hpp>

namespace game::component::state {

void HurtState::enter()
{
    playAnimation("hurt");
    stunned_timer_ = 0.0f;
    
    // 造成击退效果
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();
    auto knockback_velocity = glm::vec2(-100.0f, -150.0f); // 默认左上方击退
    
    // 根据当前精灵的朝向状态决定是否改成右上方
    if (sprite_component && sprite_component->isFlipped()) {
        knockback_velocity.x = -knockback_velocity.x;  // 变成向右
    }
    physics_component->velocity_ = knockback_velocity;
}

void HurtState::exit()
{
}

std::unique_ptr<PlayerState> HurtState::update(float delta_time, engine::core::Context& context)
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

} // namespace game::component::state
