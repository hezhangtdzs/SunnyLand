#include "jump_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include "fall_state.h"
#include <glm/glm.hpp>
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/component/sprite_component.h"
#include <cmath>

namespace game::component::state {
	void JumpState::enter() {
		playAnimation("jump");
		auto physics = player_component_->getPhysicsComponent();
		float jump_force = player_component_->getJumpForce();
		
		// 获取当前速度
		auto vel = physics->getVelocity();
		// 设置垂直速度为负的跳跃力（向上跳）
		physics->setVelocity({vel.x, -jump_force});
		
		// 确保不被判定为在地面上
		physics->setCollidedBelow(false);
	}

	void JumpState::exit() {
	}

	std::unique_ptr<PlayerState> JumpState::handleInput(engine::core::Context& context) {
		// 跳跃状态下可以左右移动 (速度系数 0.5f，即空中移动更慢)
		player_component_->processMovementInput(context, 0.5f);
		return nullptr;
	}

	std::unique_ptr<PlayerState> JumpState::update(float delta_time, engine::core::Context& context) {
		// 限制最大速度(水平方向)
		auto physics_component = player_component_->getPhysicsComponent();
		auto max_speed = player_component_->getMaxMoveSpeed();
		physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

		// 如果速度为正，切换到 FallState
		if (physics_component->velocity_.y > 0.0f) {
			return std::make_unique<FallState>(player_component_);
	}

		return nullptr;
	}
}