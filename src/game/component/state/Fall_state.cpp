#include "fall_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include "jump_state.h"
#include "climb_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/physics/physics_engine.h"
#include <glm/glm.hpp>
#include <cmath>

namespace game::component::state {

void FallState::enter() {
	playAnimation("fall");
}

void FallState::exit() {
}

std::unique_ptr<PlayerState> FallState::moveLeft(engine::core::Context& context) {
	// 下落状态下可以左右移动（空中移动更慢）
	if (auto* physics = player_component_->getPhysicsComponent()) {
		if (physics->velocity_.x > 0.0f) {
			physics->velocity_.x = 0.0f;
		}
		physics->addForce({ -player_component_->getMoveForce() * 0.5f, 0.0f });
	}
	if (auto* sprite = player_component_->getSpriteComponent()) {
		sprite->setFlipped(true);
	}
	return nullptr; // 保持在 FallState
}

std::unique_ptr<PlayerState> FallState::moveRight(engine::core::Context& context) {
	// 下落状态下可以左右移动（空中移动更慢）
	if (auto* physics = player_component_->getPhysicsComponent()) {
		if (physics->velocity_.x < 0.0f) {
			physics->velocity_.x = 0.0f;
		}
		physics->addForce({ player_component_->getMoveForce() * 0.5f, 0.0f });
	}
	if (auto* sprite = player_component_->getSpriteComponent()) {
		sprite->setFlipped(false);
	}
	return nullptr; // 保持在 FallState
}

std::unique_ptr<PlayerState> FallState::jump(engine::core::Context& context) {
	// 土狼时间跳跃
	if (player_component_->getCoyoteTimer() > 0.0f) {
		player_component_->setCoyoteTimer(0.0f);
		return std::make_unique<JumpState>(player_component_);
	}
	return nullptr;
}

std::unique_ptr<PlayerState> FallState::climbUp(engine::core::Context& context) {
	// 底部进入梯子：按上进入（玩家身体中心或脚下紧邻处在梯子列中）
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

std::unique_ptr<PlayerState> FallState::climbDown(engine::core::Context& context) {
	// 顶部进入梯子：按下进入（脚下探测到梯子）
	// 额外条件：只允许在真正下落时触发，避免在顶部边缘/空中按住下导致反复切换卡住
	auto* pc = player_component_->getPhysicsComponent();
	if (!pc || pc->velocity_.y < 0.0f || !player_component_->isOverLadder(context)) {
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
			// 进一步要求：与梯子中心列的水平偏差不能太大
			if (std::abs(ladder_center_x - center.x) > 4.0f) {
				return nullptr;
			}
			glm::vec2 pos = tc->getPosition();
			pos.x += (ladder_center_x - center.x);
			tc->setPosition(pos);
		}
	}
	return std::make_unique<ClimbState>(player_component_);
}

std::unique_ptr<PlayerState> FallState::update(float delta_time, engine::core::Context& context) {
    // 限制最大速度(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxMoveSpeed();
    physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

    // 如果下方有碰撞，则根据水平速度来决定切换到 IdleState 或 WalkState
    if (physics_component->hasCollidedBelow()) {
        if (glm::abs(physics_component->velocity_.x) < 50.0f) {
            return std::make_unique<IdleState>(player_component_);
        }
        else {
            return std::make_unique<WalkState>(player_component_);
        }
    }
    return nullptr;
}

} // namespace game::component::state
