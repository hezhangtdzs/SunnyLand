#pragma once
#include "component.h"
#include <glm/vec2.hpp>
#include "../utils/alignment.h"
#include "../utils/math.h"
#include <memory>
namespace engine {
	namespace physics {
		class Collider;
	}
	namespace object {
		class GameObject;
	}
}
namespace engine::component {
	class TransformComponent;
	class ColliderComponent final: public Component {
		friend class engine::object::GameObject;
	private:
		TransformComponent* transform_component_{ nullptr }; ///< 指向变换组件的指针
		std::unique_ptr<engine::physics::Collider> collider_{ nullptr }; ///< 指向碰撞体的指针

		glm::vec2 offset_{ 0.0f, 0.0f }; ///< 碰撞体相对于变换组件位置的偏移
		engine::utils::Alignment alignment_{ engine::utils::Alignment::CENTER }; ///< 碰撞体对齐方式
		bool is_trigger_{ false }; ///< 是否为触发器（Trigger）
		bool is_active_{ true }; ///< 碰撞体是否启用

	public:
		explicit ColliderComponent(std::unique_ptr<engine::physics::Collider> collider, engine::utils::Alignment alignment, bool is_trigger=true,bool is_active = true);
		~ColliderComponent() override = default;
		ColliderComponent(const ColliderComponent&) = delete;
		ColliderComponent& operator=(const ColliderComponent&) = delete;
		ColliderComponent(ColliderComponent&&) = delete;
		ColliderComponent& operator=(ColliderComponent&&) = delete;
		
		// 设置器/获取器
		engine::utils::Rect getWorldAABB() const;
		const TransformComponent* getTransform() const { return transform_component_; }
		void setOffset(const glm::vec2& offset) { offset_ = offset; }
		const glm::vec2& getOffset() const { return offset_; }
		void setAlignment(engine::utils::Alignment alignment);
		engine::utils::Alignment getAlignment() const { return alignment_; }
		void setIsTrigger(bool is_trigger) { is_trigger_ = is_trigger; }
		bool getIsTrigger() const { return is_trigger_; }
		void setIsActive(bool is_active) { is_active_ = is_active; }
		bool getIsActive() const { return is_active_; }
		engine::physics::Collider* getCollider() const { return collider_.get(); }



	private:
		void init() override;
		void update(float deltaTime, engine::core::Context& context) override{}
		void updateOffset();
	};
}