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
	void WalkState::enter() {
		playAnimation("walk");
	}

	void WalkState::exit() {
	}

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
}