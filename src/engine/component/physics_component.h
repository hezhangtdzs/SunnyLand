#pragma once
#include "component.h"
#include <glm/vec2.hpp>
#include <algorithm>
namespace engine {
	namespace object {
		class GameObject;
	}
}
namespace engine::physics {
	class PhysicsEngine;
}
namespace engine::component {
	class TransformComponent;
	class PhysicsComponent : public Component {
		friend class engine::object::GameObject;
	public:
		glm::vec2 velocity_{ 0.0f, 0.0f };    ///< 物体的速度向量（单位：单位/秒）
	private:
		engine::physics::PhysicsEngine* physics_engine_{ nullptr }; ///< 指向物理引擎的指针
		TransformComponent* transform_component_{ nullptr }; ///< 指向变换组件的指针

		glm::vec2 force_{ 0.0f, 0.0f };       ///< 作用在物体上的总力（单位：单位·质量/秒²）
		float mass_{ 1.0f };                  ///< 物体的质量（单位：质量单位）
		bool use_gravity_{ true };          ///< 是否受重力影响
		bool enable_{ true };              ///< 物理模拟是否启用


		bool collided_below_ = false;
		bool collided_above_ = false;
		bool collided_left_ = false;
		bool collided_right_ = false;
		bool is_climbing_ = false;
		float suppress_snap_timer_ = 0.0f;
	public:
		PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity = true, float mass = 1.0f);
		~PhysicsComponent();
		PhysicsComponent(const PhysicsComponent&) = delete;
		PhysicsComponent& operator=(const PhysicsComponent&) = delete;
		PhysicsComponent(PhysicsComponent&&) = delete;
		PhysicsComponent& operator=(PhysicsComponent&&) = delete;

		// PhysicsEngine使用的物理方法
		void addForce(const glm::vec2& force) { if (enable_) force_ += force; }    ///< @brief 添加力
		void clearForce() { force_ = { 0.0f, 0.0f }; }                                ///< @brief 清除力
		const glm::vec2& getForce() const { return force_; }                        ///< @brief 获取当前力
		float getMass() const { return mass_; }                                     ///< @brief 获取质量
		bool isEnabled() const { return enable_; }                                 ///< @brief 获取组件是否启用
		bool isUseGravity() const { return use_gravity_; }                          ///< @brief 获取组件是否受重力影响

		// 设置器/获取器
		void setEnabled(bool enabled) { enable_ = enabled; }                       ///< @brief 设置组件是否启用
		void setMass(float mass) { mass_ = (mass > 0.0f) ? mass : 1.0f; }          ///< @brief 设置质量，质量必须为正
		void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }        ///< @brief 设置组件是否受重力影响
		void setClimbing(bool is_climbing) { is_climbing_ = is_climbing; }          ///< @brief 设置是否正在攀爬
		bool isClimbing() const { return is_climbing_; }                            ///< @brief 获取是否正在攀爬
		void suppressSnapFor(float seconds) { suppress_snap_timer_ = (seconds > 0.0f) ? seconds : 0.0f; }
		bool isSnapSuppressed() const { return suppress_snap_timer_ > 0.0f; }
		void tickSnapSuppression(float dt) { if (suppress_snap_timer_ > 0.0f) suppress_snap_timer_ = std::max(0.0f, suppress_snap_timer_ - dt); }
		void setVelocity(const glm::vec2& velocity) { velocity_ = velocity; }       ///< @brief 设置速度
		const glm::vec2& getVelocity() const { return velocity_; }                  ///< @brief 获取当前速度
		TransformComponent* getTransform() const { return transform_component_; }             ///< @brief 获取TransformComponent指针
		void resetCollisionFlags() {
			collided_below_ = false;
			collided_above_ = false;
			collided_left_ = false;
			collided_right_ = false;
		}
		bool hasCollidedBelow() const { return collided_below_; }
		bool hasCollidedAbove() const { return collided_above_; }
		bool hasCollidedLeft() const { return collided_left_; }
		bool hasCollidedRight() const { return collided_right_; }

		void setCollidedBelow(bool collided) { collided_below_ = collided; }
		void setCollidedAbove(bool collided) { collided_above_ = collided; }
		void setCollidedLeft(bool collided) { collided_left_ = collided; }
		void setCollidedRight(bool collided) { collided_right_ = collided; }

	private:
		void init() override;
		void update(float deltaTime, engine::core::Context& context) override;
		void clean() override;
	};
}  // namespace engine::component