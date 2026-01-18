#pragma once
#include <glm/vec2.hpp>
namespace engine::physics {

	enum class ColliderType {
		NONE,
		AABB,
		CIRCLE,
		// 未来可扩展更多碰撞体类型
	};
	class Collider {
	protected:
		glm::vec2 aabb_size_{ 0.0f, 0.0f }; //最小包围盒尺寸
	public:
		virtual ~Collider() = default;
		virtual ColliderType getType() const = 0;
		const glm::vec2& getAABBSize() const { return aabb_size_; }
		void setAABBSize(const glm::vec2& size) { aabb_size_ = std::move(size); }
};
	class AABBCollider : public Collider {
	private:
		glm::vec2 size_{ 0.0f, 0.0f };
	public:
		AABBCollider(const glm::vec2& size) : size_(std::move(size)) {
			setAABBSize(size_);
		}
		~AABBCollider() override = default;
		ColliderType getType() const override { return ColliderType::AABB; }
		const glm::vec2& getSize() const { return size_; }
		void setSize(const glm::vec2& size) { size_ = std::move(size);}
	};
	class CircleCollider : public Collider {
	private:
		float radius_{ 0.0f };
	public:
		CircleCollider(float radius) : radius_(std::move(radius)) {
			setAABBSize({ radius_ * 2.0f, radius_ * 2.0f });
		}
		~CircleCollider() override = default;
		ColliderType getType() const override { return ColliderType::CIRCLE; }
		float getRadius() const { return radius_; }
		void setRadius(float radius) { radius_ = std::move(radius); }
	};
}  // namespace engine::physics