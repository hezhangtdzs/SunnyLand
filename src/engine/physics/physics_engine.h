#pragma once
#include <vector>
#include <glm/vec2.hpp>
namespace engine {
	namespace object {
		class GameObject;
	}
	namespace component {
		class PhysicsComponent;
	}
}
namespace engine::physics {
	/**
	 * @class PhysicsEngine
	 * @brief 物理引擎类，负责管理和更新物理组件
	 * 
	 * 该类处理所有物理相关的计算和更新，包括力的应用、速度和位置的更新等。
	 */
	class PhysicsEngine {
	private:
		std::vector<component::PhysicsComponent*> physics_components_;
		std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>> collision_pairs_;

		glm::vec2 gravity_ = { 0.0f, 980.0f };
		float max_speed_ = 5000.0f;
	public:
		/**
		 * @brief 更新所有物理组件
		 * 
		 * @param delta_time 时间增量（单位：秒）
		 */
		PhysicsEngine() = default;
		// 禁止拷贝和移动
		PhysicsEngine(const PhysicsEngine&) = delete;
		PhysicsEngine& operator=(const PhysicsEngine&) = delete;
		PhysicsEngine(PhysicsEngine&&) = delete;
		PhysicsEngine& operator=(PhysicsEngine&&) = delete;

		void registerPhysicsComponent(component::PhysicsComponent* physics_component);
		void unregisterPhysicsComponent(component::PhysicsComponent* physics_component);
		const auto& getCollisionPairs() const { return collision_pairs_; };

		void update(float delta_time);


		void setGravity(const glm::vec2& gravity) { gravity_ = gravity; }
		const glm::vec2& getGravity() const { return gravity_; }
		void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
		float getMaxSpeed() const { return max_speed_; }
	private:
		// 新增：碰撞检测循环
		void checkObjectCollisions();
	};
}  // namespace engine::physics