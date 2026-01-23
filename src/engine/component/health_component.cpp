#include "health_component.h"
#include "../object/game_object.h"
#include<spdlog/spdlog.h>
bool engine::component::HealthComponent::takeDamage(int damage)
{
	if (damage <= 0 || !isAlive() || isInvincible()) return false;
	currentHealth_ -= damage;
	currentHealth_ = glm::max(0, currentHealth_);
	if (isAlive() && invincibility_duration_ > 0.0f)
	{
		invincibility_timer_ = invincibility_duration_;
	}
	spdlog::info("GameObject [{}] took {} damage, current health: {}/{}", owner_->getName(), damage, currentHealth_, maxHealth_);
	return true;

}

void engine::component::HealthComponent::heal(int amount)
{
	if (amount <= 0 || !isAlive()) return;
	currentHealth_ += amount;
	currentHealth_ = glm::min(currentHealth_, maxHealth_);
	spdlog::info("GameObject [{}] healed {} health, current health: {}/{}", owner_->getName(), amount, currentHealth_, maxHealth_);
}

void engine::component::HealthComponent::update(float deltaTime, engine::core::Context& context)
{
	if (invincibility_timer_ > 0.0f) {
		invincibility_timer_ -= deltaTime;
		if (invincibility_timer_ < 0.0f) {
			invincibility_timer_ = 0.0f;
		}
	}
}
