#include "idle_state.h"
#include "fall_state.h"
#include "jump_state.h"
#include "walk_state.h"
#include "climb_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/physics/physics_engine.h"
#include <spdlog/spdlog.h>
#include <cmath>

namespace game::component::state {

void IdleState::enter()
{
	playAnimation("idle");
}

void IdleState::exit()
{
}

std::unique_ptr<PlayerState> IdleState::moveLeft(engine::core::Context& context)
{
	// 从待机状态开始向左移动，切换到行走状态
	return std::make_unique<WalkState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::moveRight(engine::core::Context& context)
{
	// 从待机状态开始向右移动，切换到行走状态
	return std::make_unique<WalkState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::jump(engine::core::Context& context)
{
	// 播放跳跃音效
	if (auto* audio = player_component_->getOwner()->getComponent<engine::component::AudioComponent>()) {
		audio->playSound("jump");
	}
	return std::make_unique<JumpState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::climbUp(engine::core::Context& context)
{
	// 底部进入梯子：允许按上进入（玩家身体中心或脚下紧邻处在梯子列中）
	if (auto* cc = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>()) {
		auto aabb = cc->getWorldAABB();
		glm::vec2 center = aabb.position + aabb.size * 0.5f;
		glm::vec2 feet(center.x, aabb.position.y + aabb.size.y - 2.0f);
		auto& pe = context.getPhysicsEngine();
		if (pe.getTileTypeAt(center) == engine::component::TileType::LADDER || 
		    pe.getTileTypeAt(feet) == engine::component::TileType::LADDER) {
			return std::make_unique<ClimbState>(player_component_);
		}
	}
	return nullptr;
}

std::unique_ptr<PlayerState> IdleState::climbDown(engine::core::Context& context)
{
	// 顶部进入梯子：只允许按下进入（从平台边缘向下抓梯子）
	if (!player_component_->isOverLadder(context)) {
		return nullptr;
	}
	
	// 进入攀爬时将玩家水平吸附到梯子列中心
	if (auto* tc = player_component_->getTransformComponent()) {
		if (auto* cc = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>()) {
			auto aabb = cc->getWorldAABB();
			glm::vec2 center = aabb.position + aabb.size * 0.5f;
			float ladder_center_x = 0.0f;
			if (!context.getPhysicsEngine().tryGetLadderColumnCenterX(
				glm::vec2(center.x, aabb.position.y + aabb.size.y + 12.0f), ladder_center_x)) {
				return nullptr;
			}
			{
				glm::vec2 pos = tc->getPosition();
				pos.x += (ladder_center_x - center.x);
				tc->setPosition(pos);
			}
		}
	}
	return std::make_unique<ClimbState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::update(float delta_time, engine::core::Context& context)
{
	auto physics_component = player_component_->getPhysicsComponent();
	
	// 如果下方没有碰撞，则切换到 FallState
	if (!physics_component->hasCollidedBelow()) {
		player_component_->setCoyoteTimer(0.12f);
		return std::make_unique<FallState>(player_component_);
	}

	// 应用摩擦力(水平方向)并捕捉微小速度到 0
	float friction_factor = player_component_->getFriction();
	physics_component->velocity_.x *= friction_factor;
	
	if (std::abs(physics_component->velocity_.x) < 0.1f) {
		physics_component->velocity_.x = 0.0f;
	}

	return nullptr;
}

} // namespace game::component::state
