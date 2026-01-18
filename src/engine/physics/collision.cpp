#include "collision.h"
#include "../component/collider_component.h"
#include "../component/transform_component.h"
#include "collider.h"

namespace engine::physics::collision {

    bool checkCollision(const engine::component::ColliderComponent& a, const engine::component::ColliderComponent& b) {
        // 获取两个碰撞体及对应Transform信息
        const auto* a_collider = a.getCollider();
        const auto* b_collider = b.getCollider();
        const auto* a_transform = a.getTransform();
        const auto* b_transform = b.getTransform();

        if (a_collider == nullptr || b_collider == nullptr || a_transform == nullptr || b_transform == nullptr) {
            return false;
        }

        // 先计算最小包围盒是否碰撞，如果没有碰撞，那一定是返回false (不考虑AABB的旋转)
        const auto a_aabb = a.getWorldAABB();
        const auto b_aabb = b.getWorldAABB();
        const auto a_pos = a_aabb.position;
        const auto b_pos = b_aabb.position;
        const auto a_size = a_aabb.size;
        const auto b_size = b_aabb.size;
        if (!checkAABBOverlap(a_pos, a_size, b_pos, b_size)) {
            return false;
        }

        // --- 如果最小包围盒有碰撞，再进行更细致的判断 ---
        // AABB vs AABB, 直接返回真
        if (a_collider->getType() == engine::physics::ColliderType::AABB && b_collider->getType() == engine::physics::ColliderType::AABB) {
            return true;
        }

        // Circle vs Circle
        if (a_collider->getType() == engine::physics::ColliderType::CIRCLE && b_collider->getType() == engine::physics::ColliderType::CIRCLE)
        {
            const auto a_center = a_pos + 0.5f * a_size;
            const auto b_center = b_pos + 0.5f * b_size;

            float a_radius = 0.5f * a_size.x;
            float b_radius = 0.5f * b_size.x;
            if (const auto* ca = dynamic_cast<const engine::physics::CircleCollider*>(a_collider)) {
                a_radius = ca->getRadius();
            }
            if (const auto* cb = dynamic_cast<const engine::physics::CircleCollider*>(b_collider)) {
                b_radius = cb->getRadius();
            }
            return checkCircleOverlap(a_center, a_radius, b_center, b_radius);
        }

        // AABB vs Circle
        if (a_collider->getType() == engine::physics::ColliderType::AABB && b_collider->getType() == engine::physics::ColliderType::CIRCLE)
        {
            const auto b_center = b_pos + 0.5f * b_size;
            float b_radius = 0.5f * b_size.x;
            if (const auto* cb = dynamic_cast<const engine::physics::CircleCollider*>(b_collider)) {
                b_radius = cb->getRadius();
            }

            const auto nearest_point = glm::clamp(b_center, a_pos, a_pos + a_size);
            return checkPointInCircle(nearest_point, b_center, b_radius);
        }

        // Circle vs AABB
        if (a_collider->getType() == engine::physics::ColliderType::CIRCLE && b_collider->getType() == engine::physics::ColliderType::AABB)
        {
            const auto a_center = a_pos + 0.5f * a_size;
            float a_radius = 0.5f * a_size.x;
            if (const auto* ca = dynamic_cast<const engine::physics::CircleCollider*>(a_collider)) {
                a_radius = ca->getRadius();
            }

            const auto nearest_point = glm::clamp(a_center, b_pos, b_pos + b_size);
            return checkPointInCircle(nearest_point, a_center, a_radius);
        }

        return false;
    }

    bool checkCircleOverlap(const glm::vec2& a_center, const float a_radius, const glm::vec2& b_center, const float b_radius)
    {
        const float r = a_radius + b_radius;
        return glm::dot(a_center - b_center, a_center - b_center) <= r * r;
    }

    bool checkAABBOverlap(const glm::vec2& a_pos, const glm::vec2& a_size, const glm::vec2& b_pos, const glm::vec2& b_size) {
        // 检查两个AABB是否重叠
        // 这里将"边缘接触"也视为碰撞：仅当出现严格分离时返回 false
        if (a_pos.x + a_size.x < b_pos.x || a_pos.x > b_pos.x + b_size.x ||
            a_pos.y + a_size.y < b_pos.y || a_pos.y > b_pos.y + b_size.y) {
            return false;
        }
        return true;
    }

    bool checkRectOverlap(const engine::utils::Rect& a, const engine::utils::Rect& b) {
        return checkAABBOverlap(a.position, a.size, b.position, b.size);
    }

    bool checkPointInCircle(const glm::vec2& point, const glm::vec2& center, const float radius)
    {
        return glm::dot(point - center, point - center) <= radius * radius;
    }

} // namespace engine::physics::collision