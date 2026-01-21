#include "idle_state.h"
#include "fall_state.h"
#include "jump_state.h"
#include "walk_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/component/physics_component.h"
#include <spdlog/spdlog.h>
#include <cmath>

using namespace game::component::state;

void IdleState::enter()
{
}

void IdleState::exit()
{
}

std::unique_ptr<PlayerState> IdleState::handleInput(engine::core::Context& context)
{
	auto input = context.getInputManager();

	if (input.isActionPressed("jump")) {
		return std::make_unique<JumpState>(player_component_);
	}

	// 尝试处理移动输入，如果有输入则切换到 WalkState
	if (player_component_->processMovementInput(context, 1.0f)) {
		return std::make_unique<WalkState>(player_component_);
	}

	return nullptr;
}

std::unique_ptr<PlayerState> IdleState::update(float delta_time, engine::core::Context& context)
{
	auto physics_component = player_component_->getPhysicsComponent();
	
	// 如果下方没有碰撞，则切换到 FallState
	if (!physics_component->hasCollidedBelow()) {
		return std::make_unique<FallState>(player_component_);
	}

	// 应用摩擦力(水平方向)并捕捉微小速度到 0
	float friction_factor = player_component_->getFriction();
	// 注意：摩擦力应该是 delta_time 相关的，但这里沿用简单的乘法因子
	// 如果 friction 是 0.8 / 帧，则保持原样
	physics_component->velocity_.x *= friction_factor;
	
	if (std::abs(physics_component->velocity_.x) < 0.1f) {
		physics_component->velocity_.x = 0.0f;
	}

	return nullptr;
}
