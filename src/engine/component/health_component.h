#pragma once
#include "component.h"
#include <glm/glm.hpp>
namespace engine::component {
	class HealthComponent final :public Component {
		friend class engine::object::GameObject;
	private:
		int maxHealth_;
		int currentHealth_;
		bool is_invincible_ = false;
		float invincibility_duration_ = 2.0f;
		float invincibility_timer_ = 0.0f;
	public:
		HealthComponent(int maxHealth, float invincibility_duration = 2.0f)
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
			is_invincible_ = true;
			invincibility_duration_ = duration;
		}
		void setInvincible(bool is_invincible) {
			is_invincible_ = is_invincible;
			if (!is_invincible_) {
				invincibility_timer_ = 0.0f;
			}
		}
		//getters
		int getCurrentHealth() const { return currentHealth_; }
		int getMaxHealth() const { return maxHealth_; }
		bool isInvincible() const { return is_invincible_; }
		float getInvincibilityDuration() const { return invincibility_duration_; }

		bool takeDamage(int damage);
		void heal(int amount);
		bool isAlive() const {
			return currentHealth_ > 0;
		}
	protected:
		void update(float deltaTime, engine::core::Context& context) override;
	};

}