#pragma once
#include "component.h"
#include <glm/glm.hpp>
namespace engine::component {
	class HealthComponent final :public Component {
		friend class engine::object::GameObject;
	private:
		int maxHealth_;
		int currentHealth_;
		float invincibility_duration_ = 1.5f;
		float invincibility_timer_ = 0.0f;
	public:
		HealthComponent(int maxHealth, float invincibility_duration = 1.5f)
			: maxHealth_(maxHealth), currentHealth_(maxHealth), invincibility_duration_(invincibility_duration) {}

		
		void setMaxHealth(int maxHealth) {
			maxHealth_ = maxHealth;
			if (currentHealth_ > maxHealth_) {
				currentHealth_ = maxHealth_;
			}
		}
		void setCurrentHealth(int currentHealth) {
			currentHealth_ = glm::clamp(currentHealth, 0, maxHealth_);
		}
		void setInvincibilityDuration(float duration) {
			invincibility_duration_ = duration;
		}
		void setInvincible(float duration) {
			invincibility_timer_ = duration;
		}
		//getters
		int getCurrentHealth() const { return currentHealth_; }
		int getMaxHealth() const { return maxHealth_; }
		bool isInvincible() const { return invincibility_timer_ > 0.0f; }
		float getInvincibilityDuration() const { return invincibility_duration_; }
		float getInvincibilityTimer() const { return invincibility_timer_; }

		bool takeDamage(int damage);
		void heal(int amount);
		bool isAlive() const {
			return currentHealth_ > 0;
		}
	protected:
		void update(float deltaTime, engine::core::Context& context) override;
	};

}