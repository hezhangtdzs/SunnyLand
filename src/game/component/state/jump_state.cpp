#include "jump_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include "fall_state.h"
#include "climb_state.h"
#include <glm/glm.hpp>

#include "../player_component.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/physics/physics_engine.h"
#include "../../../engine/component/sprite_component.h"
#include <cmath>

namespace game::component::state {
	void JumpState::enter() {
		playAnimation("jump");
		// Sound playback is dispatched from the state machine input/update paths where Context is available.
		player_component_->setCoyoteTimer(0.0f);
		auto physics = player_component_->getPhysicsComponent();
		float jump_force = player_component_->getJumpForce();
		
		// 获取当前速度
		auto vel = physics->getVelocity();
		// 设置垂直速度为负的跳跃力（向上跳）
		// 斜坡上起跳时，如果正好处于贴地/嵌入修正边缘，下一帧可能被重新贴回斜坡。
		// 这里额外向上“抬”一点点，确保离开地表。
		physics->setVelocity({ vel.x, -jump_force });
		if (auto* tc = player_component_->getTransformComponent()) {
			auto pos = tc->getPosition();
			pos.y -= 1.0f;
			tc->setPosition(pos);
		}
		
		// 确保不被判定为在地面上
		physics->setCollidedBelow(false);
		// 起跳后短时间禁用斜坡/单向平台吸附，防止被 Stickiness 拉回斜坡导致“滑行跳”
		physics->suppressSnapFor(0.22f);
	}

	void JumpState::exit() {
	}

	std::unique_ptr<PlayerState> JumpState::handleInput(engine::core::Context& context) {
		auto& input = context.getInputManager();
		// JumpState 禁止进入攀爬：避免空中按下/按上在梯子顶部附近反复触发切换。
		// 梯子的进入由 Idle/Walk/Fall 处理（顶部按下，底部按上）。

		// 跳跃状态下可以左右移动 (速度系数 0.5f，即空中移动更慢)
		player_component_->processMovementInput(context, 0.5f);
		return nullptr;
	}


	std::unique_ptr<PlayerState> JumpState::update(float delta_time, engine::core::Context& context) {
		// 限制最大速度(水平方向)
		auto physics_component = player_component_->getPhysicsComponent();
		auto max_speed = player_component_->getMaxMoveSpeed();
		physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

		// 从梯子顶部跳出时，可能会落在平台边缘并很快变为“站立/行走”。
		// 仅靠 velocity.y > 0 切换到 FallState 会导致一直停留在 JumpState。
		if (physics_component->hasCollidedBelow()) {
			if (glm::abs(physics_component->velocity_.x) < 50.0f) {
				return std::make_unique<IdleState>(player_component_);
			}
			return std::make_unique<WalkState>(player_component_);
		}

		// 如果速度为正，切换到 FallState
		if (physics_component->velocity_.y > 0.0f) {
			return std::make_unique<FallState>(player_component_);
	}

		return nullptr;
	}
}