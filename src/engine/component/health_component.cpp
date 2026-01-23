#include "health_component.h"
#include "../object/game_object.h"
#include<spdlog/spdlog.h>
bool engine::component::HealthComponent::takeDamage(int damage)
{
	if (damage <= 0 || !isAlive()|| is_invincible_) return false;
	currentHealth_ -= damage;
	currentHealth_ = glm::max(0, currentHealth_);
	if (isAlive()&& invincibility_duration_ > 0.0f)
	{
		setInvincibilityDuration(invincibility_duration_);

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
	if (is_invincible_) {
		invincibility_duration_ -= deltaTime;
		if (invincibility_duration_ <= 0.0f) {
			is_invincible_ = false;
			invincibility_timer_ = 0.0f;
		}
	}
}
