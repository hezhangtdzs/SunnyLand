#include "player_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "state/idle_state.h"
#include "../../engine/input/input_manager.h" // Needed for InputManager
#include <spdlog/spdlog.h>

void game::component::PlayerComponent::setState(std::unique_ptr<state::PlayerState> new_state)
{
	if (current_state_) {
		current_state_->exit();
	}
	else {
		spdlog::info("PlayerComponent 初始状态设置：nullptr -> {}",
			new_state ? typeid(*new_state).name() : "nullptr");
	}
	current_state_ = std::move(new_state);
	if (current_state_) {
		current_state_->enter();
	}
}

void game::component::PlayerComponent::init()
{
	if(owner_) {
		transform_component_ = owner_->getComponent<engine::component::TransformComponent>();
		sprite_component_ = owner_->getComponent<engine::component::SpriteComponent>();
		physics_component_ = owner_->getComponent<engine::component::PhysicsComponent>();
	}
	else
	{
		spdlog::error("PlayerComponent 初始化失败：所属对象为空");
	}
	if (!transform_component_) {
		spdlog::error("PlayerComponent 初始化失败：缺少 TransformComponent 组件");
	}
	if (!sprite_component_) {
		spdlog::error("PlayerComponent 初始化失败：缺少 SpriteComponent 组件");
	}
	if (!physics_component_) {
		spdlog::error("PlayerComponent 初始化失败：缺少 PhysicsComponent 组件");
	}
	setState(std::make_unique<state::IdleState>(this));
}

void game::component::PlayerComponent::handleInput(engine::core::Context& context)
{
	if (current_state_) {
		auto new_state = current_state_->handleInput(context);
		if (new_state) {
			setState(std::move(new_state));
		}
	}
}

void game::component::PlayerComponent::update(float delta_time, engine::core::Context& context)
{
	if (current_state_) {
		auto new_state = current_state_->update(delta_time, context);
		if (new_state) {
			setState(std::move(new_state));
		}
	}
}

bool game::component::PlayerComponent::processMovementInput(engine::core::Context& context, float speed_scale)
{
	if (!physics_component_ || !sprite_component_) return false;

	auto& input_manager = context.getInputManager();
	bool has_input = false;

	// X轴移动逻辑 (左/右)
	if (input_manager.isActionDown("move_left")) {
		// 快速转向/刹车逻辑：如果当前速度是向右(>0)，则先归零
		if (physics_component_->velocity_.x > 0.0f) {
			physics_component_->velocity_.x = 0.0f;
		}

		physics_component_->addForce({ -move_force_ * speed_scale, 0.0f });
		sprite_component_->setFlipped(true);
		has_input = true;
	}
	else if (input_manager.isActionDown("move_right")) {
		// 快速转向/刹车逻辑：如果当前速度是向左(<0)，则先归零
		if (physics_component_->velocity_.x < 0.0f) {
			physics_component_->velocity_.x = 0.0f;
		}

		physics_component_->addForce({ move_force_ * speed_scale, 0.0f });
		sprite_component_->setFlipped(false);
		has_input = true;
	}

	return has_input;
}
