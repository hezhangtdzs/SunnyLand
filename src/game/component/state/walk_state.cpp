#include "walk_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include "fall_state.h"
#include "climb_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/physics/physics_engine.h"
#include <cmath>
#include <glm/glm.hpp>

namespace game::component::state {

/**
 * @brief 进入行走状态
 * 
 * @details 播放行走动画
 */
void WalkState::enter() {
	playAnimation("walk");
}

/**
 * @brief 退出行走状态
 * 
 * @details 行走状态退出时不需要特殊处理
 */
void WalkState::exit() {
}

/**
 * @brief 处理行走状态的输入
 * 
 * @param context 引擎上下文
 * @return std::unique_ptr<PlayerState> 新状态指针，若不发生跳转则返回 nullptr
 * @details 处理跳跃、攀爬梯子和移动输入，根据输入切换到相应的状态
 */
std::unique_ptr<PlayerState> WalkState::handleInput(engine::core::Context& context) {

	auto& input_manager = context.getInputManager();

	// 如果按下"jump"则切换到 JumpState
	if (input_manager.isActionPressed("jump")) {
		if (auto* audio = player_component_->getOwner()->getComponent<engine::component::AudioComponent>()) {
			audio->playSound("jump", context);
		}
		return std::make_unique<JumpState>(player_component_);
	}

	// 顶部进入梯子：只允许按下进入（从平台边缘向下抓梯子）。按上必须已经在梯子里才允许。
	if (input_manager.isActionDown("move_down") && player_component_->isOverLadder(context)) {
		// 进入攀爬时将玩家水平吸附到梯子列中心，避免在梯子边缘进入攀爬却不在梯子里
		if (auto* tc = player_component_->getTransformComponent()) {
			if (auto* cc = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>()) {
				auto aabb = cc->getWorldAABB();
				glm::vec2 center = aabb.position + aabb.size * 0.5f;
				float ladder_center_x = 0.0f;
				if (context.getPhysicsEngine().tryGetLadderColumnCenterX(glm::vec2(center.x, aabb.position.y + aabb.size.y + 12.0f), ladder_center_x)) {
					glm::vec2 pos = tc->getPosition();
					pos.x += (ladder_center_x - center.x);
					tc->setPosition(pos);
				}
			}
		}
		return std::make_unique<ClimbState>(player_component_);
	}
	if (input_manager.isActionDown("move_up")) {
		if (auto* cc = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>()) {
			auto aabb = cc->getWorldAABB();
			glm::vec2 center = aabb.position + aabb.size * 0.5f;
			glm::vec2 feet(center.x, aabb.position.y + aabb.size.y - 2.0f);
			auto& pe = context.getPhysicsEngine();
			if (pe.getTileTypeAt(center) == engine::component::TileType::LADDER || pe.getTileTypeAt(feet) == engine::component::TileType::LADDER) {
				return std::make_unique<ClimbState>(player_component_);
			}
		}
	}

	// 使用 helper 处理移动 (WalkState 速度系数 1.0f)
	bool moved = player_component_->processMovementInput(context, 1.0f);

	if (!moved) {
		// 如果没有按下左右移动键，则切换到 IdleState
		return std::make_unique<IdleState>(player_component_);
	}

	return nullptr;
}

/**
 * @brief 更新行走状态逻辑
 * 
 * @param delta_time 时间增量（秒）
 * @param context 引擎上下文
 * @return std::unique_ptr<PlayerState> 新状态指针，若不发生跳转则返回 nullptr
 * @details 限制水平速度不超过最大速度，检查是否有下方碰撞，如果没有则切换到下落状态
 */
std::unique_ptr<PlayerState> WalkState::update(float delta_time, engine::core::Context& context) {
	auto physics_component = player_component_->getPhysicsComponent();
	auto max_speed = player_component_->getMaxMoveSpeed();
	physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

	// 如果下方没有碰撞，则切换到 FallState
	if (!physics_component->hasCollidedBelow()) {
		player_component_->setCoyoteTimer(0.12f);
		return std::make_unique<FallState>(player_component_);
	}

	return nullptr;
}

} // namespace game::component::state