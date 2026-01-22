#include "walk_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include "fall_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include <cmath>
#include <glm/glm.hpp>

namespace game::component::state {
	void WalkState::enter() {
		playAnimation("walk");
	}

	void WalkState::exit() {
	}

	std::unique_ptr<PlayerState> WalkState::handleInput(engine::core::Context& context) {

		auto input_manager = context.getInputManager();

		// 如果按下"jump"则切换到 JumpState
		if (input_manager.isActionPressed("jump")) {
			return std::make_unique<JumpState>(player_component_);
		}

		// 使用 helper 处理移动 (WalkState 速度系数 1.0f)
		bool moved = player_component_->processMovementInput(context, 1.0f);

		if (!moved) {
			// 如果没有按下左右移动键，则切换到 IdleState
			return std::make_unique<IdleState>(player_component_);
		}
		return nullptr;
	}

	std::unique_ptr<PlayerState> WalkState::update(float delta_time, engine::core::Context& context) {
		auto physics_component = player_component_->getPhysicsComponent();
		auto max_speed = player_component_->getMaxMoveSpeed();
		physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

		// 如果下方没有碰撞，则切换到 FallState
		if (!physics_component->hasCollidedBelow()) {
			return std::make_unique<FallState>(player_component_);
		}

		return nullptr;
	}
}