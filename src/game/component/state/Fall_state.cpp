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
#include "../../../engine/physics/physics_engine.h"
#include <glm/glm.hpp>
#include "../../../engine/input/input_manager.h"
#include <cmath>

namespace game::component::state {

/**
 * @brief 进入下落状态
 * 
 * @details 播放下落动画
 */
void FallState::enter() {
	playAnimation("fall");
}

/**
 * @brief 退出下落状态
 * 
 * @details 下落状态退出时不需要特殊处理
 */
void FallState::exit() {
}

/**
 * @brief 处理下落状态的输入
 * 
 * @param context 引擎上下文
 * @return std::unique_ptr<PlayerState> 新状态指针，若不发生跳转则返回 nullptr
 * @details 处理跳跃（包括土狼时间）、攀爬梯子和左右移动输入
 */
std::unique_ptr<PlayerState> FallState::handleInput(engine::core::Context& context) {
	auto& input = context.getInputManager();

	if (input.isActionPressed("jump") && player_component_->getCoyoteTimer() > 0.0f) {
		player_component_->setCoyoteTimer(0.0f);
		return std::make_unique<JumpState>(player_component_);
	}

	// 顶部：按下进入（脚下探测到梯子）
	// 额外条件：只允许在真正下落时触发，避免在顶部边缘/空中按住下导致反复切换卡住。
	auto* pc = player_component_->getPhysicsComponent();
	if (input.isActionDown("move_down") && pc && pc->velocity_.y >= 0.0f && player_component_->isOverLadder(context)) {
		// 进入攀爬时将玩家水平吸附到梯子列中心，避免在梯子边缘抓到梯子但不对齐
		if (auto* tc = player_component_->getTransformComponent()) {
			if (auto* cc = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>()) {
				auto aabb = cc->getWorldAABB();
				glm::vec2 center = aabb.position + aabb.size * 0.5f;
				float ladder_center_x = 0.0f;
				if (!context.getPhysicsEngine().tryGetLadderColumnCenterX(glm::vec2(center.x, aabb.position.y + aabb.size.y + 12.0f), ladder_center_x)) {
					return nullptr;
				}
				// 进一步要求：与梯子中心列的水平偏差不能太大，否则认为只是擦到探测范围
				if (std::abs(ladder_center_x - center.x) > 4.0f) {
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
	// 底部：按上进入（中心或脚部在梯子列里）
	if (input.isActionDown("move_up")) {
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

	// 下落状态下可以左右移动 (速度系数 0.5f)
	player_component_->processMovementInput(context, 0.5f);
	return nullptr;
}

/**
 * @brief 更新下落状态逻辑
 * 
 * @param delta_time 时间增量（秒）
 * @param context 引擎上下文
 * @return std::unique_ptr<PlayerState> 新状态指针，若不发生跳转则返回 nullptr
 * @details 限制水平速度，检查是否碰撞到地面，根据水平速度切换到相应的状态
 */
std::unique_ptr<PlayerState> FallState::update(float delta_time, engine::core::Context& context) {
    // 限制最大速度(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxMoveSpeed();
    physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

    // 如果下方有碰撞，则根据水平速度来决定 切换到 IdleState 或 WalkState
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