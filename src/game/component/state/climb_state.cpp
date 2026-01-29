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
#include "../../../engine/render/sprite.h"

#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/physics/physics_engine.h"
#include <glm/glm.hpp>

namespace game::component::state {

/**
 * @brief 进入攀爬状态
 * 
 * @details 播放攀爬动画，禁用重力，设置攀爬状态并重置速度
 */
void ClimbState::enter() {
	playAnimation("climb");
	auto* pc = player_component_->getPhysicsComponent();
	if (pc) {
		pc->setUseGravity(false);
		pc->setClimbing(true);
		pc->velocity_ = glm::vec2(0.0f);
	}
}

/**
 * @brief 退出攀爬状态
 * 
 * @details 启用重力，禁用攀爬状态
 */
void ClimbState::exit() {
	auto* pc = player_component_->getPhysicsComponent();
	if (pc) {
		pc->setUseGravity(true);
		pc->setClimbing(false);
	}
}

/**
 * @brief 处理攀爬状态的输入
 * 
 * @param context 引擎上下文
 * @return std::unique_ptr<PlayerState> 新状态指针，若不发生跳转则返回 nullptr
 * @details 处理跳跃输入和攀爬移动，根据输入设置攀爬速度和方向
 */
std::unique_ptr<PlayerState> ClimbState::handleInput(engine::core::Context& context) {
	auto& input = context.getInputManager();
	auto* pc = player_component_->getPhysicsComponent();

	if (input.isActionPressed("jump")) {
		return std::make_unique<JumpState>(player_component_);
	}

	if (pc) {
		float vy = 0.0f;
		if (input.isActionDown("move_up")) {
			vy = -climb_speed_;
		}
		else if (input.isActionDown("move_down")) {
			vy = climb_speed_;
		}
		pc->velocity_.y = vy;

		// 在攀爬时允许微量的左右移动 (直接给速度，避免力衰减导致的动画抖动)
		float vx = 0.0f;
		if (input.isActionDown("move_left")) {
			vx = -climb_speed_ * 0.4f;
			auto* sprite = player_component_->getSpriteComponent();
			if (sprite) sprite->setFlipped(true);
		}
		else if (input.isActionDown("move_right")) {
			vx = climb_speed_ * 0.4f;
			auto* sprite = player_component_->getSpriteComponent();
			if (sprite) sprite->setFlipped(false);
		}
		pc->velocity_.x = vx;
	}

	return nullptr;
}

/**
 * @brief 更新攀爬状态逻辑
 * 
 * @param delta_time 时间增量（秒）
 * @param context 引擎上下文
 * @return std::unique_ptr<PlayerState> 新状态指针，若不发生跳转则返回 nullptr
 * @details 检测梯子接触情况，处理登顶、落地和离开梯子的逻辑，控制攀爬动画的播放状态
 */
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

	// 关键采样点：头 / 身体中心 / 脚下(向下多探测一段) / 脚上(为了解决最底部按上不上去)
	const float head_y = aabb.position.y;
	const float center_y = center.y;
	const float feet_probe_y = aabb.position.y + aabb.size.y + 12.0f;
	const float above_feet_y = aabb.position.y + aabb.size.y - 2.0f;

	bool head_on_ladder = isLadderAt(head_y);
	bool center_on_ladder = isLadderAt(center_y);
	bool feet_on_ladder = isLadderAt(feet_probe_y);
	bool above_feet_on_ladder = isLadderAt(above_feet_y);

	// 1) 底部修复：最底部按上时，feet_probe 可能已经离开梯子，但脚上方仍然在梯子列里
	// 只要脚上方还在梯子上，就继续保持攀爬
	if (!feet_on_ladder && above_feet_on_ladder) {
		feet_on_ladder = true;
	}

	// 2) 离开梯子：这时才退出
	if (!head_on_ladder && !center_on_ladder && !feet_on_ladder) {
		if (pc->hasCollidedBelow()) return std::make_unique<IdleState>(player_component_);
		return std::make_unique<FallState>(player_component_);
	}

	// 3) 登顶：向上爬并且身体中心离开梯子，但脚附近仍处于梯子列里，吸附并退出
	if (pc->velocity_.y < 0 && !center_on_ladder && feet_on_ladder) {
		if (!isLadderAt(aabb.position.y + aabb.size.y - 6.0f)) {
			pc->velocity_.y = 0;
			transform->translate({ 0.0f, -6.0f });
			return std::make_unique<IdleState>(player_component_);
		}
	}

	// 4) 落地：只要已经碰到底部地面并且玩家没有在向上爬，就退出（避免卡在底部仍然是 climb ）
	if (pc->hasCollidedBelow() && pc->velocity_.y >= 0) {
		return std::make_unique<IdleState>(player_component_);
	}

	// 动画控制：只用玩家输入速度判断，避免由于浮点残留导致持续播放
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