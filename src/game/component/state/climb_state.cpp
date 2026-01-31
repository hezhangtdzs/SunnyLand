#include "climb_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include "fall_state.h"
#include "../player_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/physics/physics_engine.h"
#include <glm/glm.hpp>

namespace game::component::state {

void ClimbState::enter() {
	playAnimation("climb");
	auto* pc = player_component_->getPhysicsComponent();
	if (pc) {
		pc->setUseGravity(false);
		pc->setClimbing(true);
		pc->velocity_ = glm::vec2(0.0f);
	}
}

void ClimbState::exit() {
	auto* pc = player_component_->getPhysicsComponent();
	if (pc) {
		pc->setUseGravity(true);
		pc->setClimbing(false);
	}
}

std::unique_ptr<PlayerState> ClimbState::moveLeft(engine::core::Context& context) {
	// 在攀爬时允许微量的左右移动
	if (auto* pc = player_component_->getPhysicsComponent()) {
		pc->velocity_.x = -climb_speed_ * 0.4f;
	}
	if (auto* sprite = player_component_->getSpriteComponent()) {
		sprite->setFlipped(true);
	}
	return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::moveRight(engine::core::Context& context) {
	// 在攀爬时允许微量的左右移动
	if (auto* pc = player_component_->getPhysicsComponent()) {
		pc->velocity_.x = climb_speed_ * 0.4f;
	}
	if (auto* sprite = player_component_->getSpriteComponent()) {
		sprite->setFlipped(false);
	}
	return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::jump(engine::core::Context& context) {
	// 攀爬状态下跳跃，退出攀爬
	return std::make_unique<JumpState>(player_component_);
}

std::unique_ptr<PlayerState> ClimbState::climbUp(engine::core::Context& context) {
	// 向上攀爬
	if (auto* pc = player_component_->getPhysicsComponent()) {
		pc->velocity_.y = -climb_speed_;
	}
	return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::climbDown(engine::core::Context& context) {
	// 向下攀爬
	if (auto* pc = player_component_->getPhysicsComponent()) {
		pc->velocity_.y = climb_speed_;
	}
	return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::stopMove(engine::core::Context& context) {
	// 停止攀爬移动
	if (auto* pc = player_component_->getPhysicsComponent()) {
		pc->velocity_ = glm::vec2(0.0f);
	}
	return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::update(float delta_time, engine::core::Context& context) {
	auto* pc = player_component_->getPhysicsComponent();
	auto* transform = player_component_->getTransformComponent();
	auto* collider = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>();
	
	if (!pc || !transform || !collider) return nullptr;

	auto aabb = collider->getWorldAABB();
	glm::vec2 center = aabb.position + aabb.size * 0.5f;

	auto& physics_engine = context.getPhysicsEngine();
	auto tileAt = [&](float y) {
		return physics_engine.getTileTypeAt(glm::vec2(center.x, y));
	};
	auto isLadderAt = [&](float y) {
		return tileAt(y) == engine::component::TileType::LADDER;
	};

	// 关键采样点：头 / 身体中心 / 脚下(向下多探测一段) / 脚上
	const float head_y = aabb.position.y;
	const float center_y = center.y;
	const float feet_probe_y = aabb.position.y + aabb.size.y + 12.0f;
	const float above_feet_y = aabb.position.y + aabb.size.y - 2.0f;

	bool head_on_ladder = isLadderAt(head_y);
	bool center_on_ladder = isLadderAt(center_y);
	bool feet_on_ladder = isLadderAt(feet_probe_y);
	bool above_feet_on_ladder = isLadderAt(above_feet_y);

	// 底部修复：最底部按上时，feet_probe 可能已经离开梯子，但脚上方仍然在梯子列里
	if (!feet_on_ladder && above_feet_on_ladder) {
		feet_on_ladder = true;
	}

	// 离开梯子：这时才退出
	if (!head_on_ladder && !center_on_ladder && !feet_on_ladder) {
		if (pc->hasCollidedBelow()) return std::make_unique<IdleState>(player_component_);
		return std::make_unique<FallState>(player_component_);
	}

	// 登顶：向上爬并且身体中心离开梯子，但脚附近仍处于梯子列里，吸附并退出
	if (pc->velocity_.y < 0 && !center_on_ladder && feet_on_ladder) {
		if (!isLadderAt(aabb.position.y + aabb.size.y - 6.0f)) {
			pc->velocity_.y = 0;
			transform->translate({ 0.0f, -6.0f });
			return std::make_unique<IdleState>(player_component_);
		}
	}

	// 落地：只要已经碰到底部地面并且玩家没有在向上爬，就退出
	if (pc->hasCollidedBelow() && pc->velocity_.y >= 0) {
		return std::make_unique<IdleState>(player_component_);
	}

	// 动画控制：只用玩家输入速度判断
	auto* ac = player_component_->getAnimationComponent();
	if (ac) {
		if (glm::abs(pc->velocity_.x) < 0.1f && glm::abs(pc->velocity_.y) < 0.1f) {
			ac->setPlaying(false);
		}
		else {
			ac->setPlaying(true);
		}
	}

	return nullptr;
}

} // namespace game::component::state
