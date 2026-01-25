#include "player_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/health_component.h"
#include "state/idle_state.h"
#include "state/hurt_state.h"
#include "state/dead_state.h"
#include "../../engine/input/input_manager.h" // Needed for InputManager
#include "../../engine/component/collider_component.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/core/context.h"
#include <spdlog/spdlog.h>

engine::component::AnimationComponent* game::component::PlayerComponent::getAnimationComponent() const
{
	return animation_component_;
}

bool game::component::PlayerComponent::takeDamage(int damage)
{
	if(!is_dead_ && health_component_ && health_component_->isAlive()) {
		spdlog::info("PlayerComponent 收到伤害：{}", damage);
	}
	if (health_component_) {
		bool success = health_component_->takeDamage(damage);
		if (success) {
			if (health_component_->isAlive()) {
				spdlog::info("PlayerComponent 受伤，当前生命值：{}/{}",
					health_component_->getCurrentHealth(),
					health_component_->getMaxHealth());
					setState(std::make_unique<state::HurtState>(this));
			}
			else {
				spdlog::info("PlayerComponent 死亡。");
				is_dead_ = true;
				setState(std::make_unique<state::DeadState>(this));	
			}
		}
	}
	return false;
}

bool game::component::PlayerComponent::isOverLadder(engine::core::Context& context) const
{
	auto* collider = owner_->getComponent<engine::component::ColliderComponent>();
	if (!collider) return false;
	auto aabb = collider->getWorldAABB();

	auto& physics_engine = context.getPhysicsEngine();

	// 进入攀爬只允许“玩家中心轴线正下方”是梯子。
	// 但顶部平台/轴分离碰撞会导致脚底探测点刚好落在平台 tile 上，因此这里做一个小范围向下采样。
	glm::vec2 center = aabb.position + aabb.size * 0.5f;

	// 从顶部向下进入：在脚底下方做范围探测（最多 1 格）
	const float base_y = aabb.position.y + aabb.size.y;
	for (float dy = 2.0f; dy <= 18.0f; dy += 4.0f) {
		if (physics_engine.getTileTypeAt(glm::vec2(center.x, base_y + dy)) == engine::component::TileType::LADDER) {
			return true;
		}
	}

	return false;
}


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
		animation_component_ = owner_->getComponent<engine::component::AnimationComponent>();
		health_component_ = owner_->getComponent<engine::component::HealthComponent>();
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
	if (!health_component_) {
		spdlog::error("PlayerComponent 初始化失败：缺少 HealthComponent 组件");
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

	if (coyote_timer_ > 0.0f) {
		coyote_timer_ -= delta_time;
	}

	// 处理受伤后的闪烁效果
	if (health_component_ && health_component_->isInvincible()) {
		// 使用无敌剩余时间来计算闪烁频率
		bool visible = static_cast<int>(health_component_->getInvincibilityTimer() * 15) % 2 == 0;
		if (sprite_component_) {
			sprite_component_->setHidden(!visible);
		}
	}
	else if (sprite_component_ && sprite_component_->isHidden()) {
		sprite_component_->setHidden(false);
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
