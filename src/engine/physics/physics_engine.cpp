#include "physics_engine.h"
#include <spdlog/spdlog.h>
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../component/collider_component.h"
#include "../component/tilelayer_component.h"
#include "collision.h"
#include "../object/game_object.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
void engine::physics::PhysicsEngine::registerPhysicsComponent(component::PhysicsComponent* physics_component)
{
	physics_components_.push_back(physics_component);
	spdlog::info("物理组件注册 {}", static_cast<void*>(physics_component));
}

void engine::physics::PhysicsEngine::registerCollisionLayer(component::TileLayerComponent* tilelayer_component)
{
	tilelayer_component->setPhysicsEngine(this);
	tilelayer_components_.push_back(tilelayer_component);
	spdlog::info("瓦片图层组件注册 {}", static_cast<void*>(tilelayer_component));
}

void engine::physics::PhysicsEngine::unregisterPhysicsComponent(component::PhysicsComponent* physics_component)
{
	physics_components_.erase(std::remove(physics_components_.begin(), physics_components_.end(), physics_component), physics_components_.end());
	spdlog::info("物理组件注册注销 {}", static_cast<void*>(physics_component));
}

void engine::physics::PhysicsEngine::unregisterCollisionLayer(component::TileLayerComponent* tilelayer_component)
{
	tilelayer_components_.erase(std::remove(tilelayer_components_.begin(), tilelayer_components_.end(), tilelayer_component), tilelayer_components_.end());
	spdlog::info("瓦片图层组件注册注销 {}", static_cast<void*>(tilelayer_component));
}

void engine::physics::PhysicsEngine::update(float delta_time)
{
	collision_pairs_.clear();
	// 防止卡顿/断点导致 dt 过大，从而一帧内位移过大直接飞出镜头
	const float dt = std::clamp(delta_time, 0.0f, 1.0f / 30.0f);
    for (auto* pc : physics_components_) {
        if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
            continue;
        }

		const float mass = pc->getMass();
		if (!(mass > 0.0f) || !std::isfinite(mass)) {
			continue;
		}

        // 应用重力 (如果组件受重力影响)：F = g * m
        if (pc->isUseGravity()) {
            pc->addForce(gravity_ * pc->getMass());
        }
        /* 还可以添加其它力影响，比如风力、摩擦力等，目前不考虑 */

        // 更新速度： v += a * dt，其中 a = F / m
        pc->velocity_ += (pc->getForce() / mass) * dt;
        pc->clearForce(); // 清除当前帧的力

		// 限制最大速度：v = min(v, max_speed)
		pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);


		resolveTileCollisions(pc, dt);

	}
	checkObjectCollisions();
}

void engine::physics::PhysicsEngine::checkObjectCollisions()
{
	// 基于 ColliderComponent 的碰撞检测
	for (size_t i = 0; i < physics_components_.size(); ++i) {
		auto* pcA = physics_components_[i];
		if (!pcA || !pcA->isEnabled()) continue;
		auto* ownerA = pcA->getOwner();
		if (!ownerA) continue;
		auto* colA = ownerA->getComponent<engine::component::ColliderComponent>();
		if (!colA || !colA->getIsActive()) continue;
		for (size_t j = i + 1; j < physics_components_.size(); ++j) {
			auto* pcB = physics_components_[j];
			if (!pcB || !pcB->isEnabled()) continue;
			auto* ownerB = pcB->getOwner();
			if (!ownerB) continue;
			auto* colB = ownerB->getComponent<engine::component::ColliderComponent>();
			if (!colB || !colB->getIsActive()) continue;

			if (engine::physics::collision::checkCollision(*colA, *colB)) {
				collision_pairs_.emplace_back(ownerA, ownerB);
				spdlog::info("碰撞检测: 对象 {} 与 对象 {} 碰撞", ownerA->getName(), ownerB->getName());
			}
		}
	}
}

void engine::physics::PhysicsEngine::resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time)
{
    using engine::component::ColliderComponent;
    using engine::component::TileType;
    using engine::component::TransformComponent;

    if (!pc || !pc->isEnabled()) {
        return;
    }

    auto* obj = pc->getOwner();
    if (!obj) {
        return;
    }
    auto* tc = obj->getComponent<TransformComponent>();
    auto* cc = obj->getComponent<ColliderComponent>();
    if (!tc || !cc || !cc->getIsActive() || cc->getIsTrigger()) {
        return;
    }

    const glm::vec2 collider_offset = cc->getOffset();
    const glm::vec2 collider_size = cc->getWorldAABB().size;

    glm::vec2 aabb_pos = tc->getPosition() + collider_offset;

    const glm::vec2 ds = pc->velocity_ * delta_time;
    const float eps = 0.001f;

    // 输出调试信息
    // std::cout << "Current pos: " << current_pos.x << ", " << current_pos.y << std::endl;
    // std::cout << "Collider offset: " << collider_offset.x << ", " << collider_offset.y << std::endl;
    // std::cout << "Current AABB pos: " << current_aabb_pos.x << ", " << current_aabb_pos.y << std::endl;
    // std::cout << "Velocity: " << pc->velocity_.x << ", " << pc->velocity_.y << std::endl;
    // std::cout << "Delta time: " << delta_time << std::endl;

    for (auto* layer : tilelayer_components_) {
        if (!layer || layer->isHidden()) {
            continue;
        }
        const glm::vec2 tile_size(layer->getTileSize());
        if (tile_size.x <= 0.0f || tile_size.y <= 0.0f) {
            continue;
        }

        glm::vec2 layer_offset = layer->getOffset();
        if (auto* layer_owner = layer->getOwner()) {
            if (auto* layer_tc = layer_owner->getComponent<TransformComponent>()) {
                layer_offset += layer_tc->getPosition();
            }
        }

        auto isSolid = [&](int tx, int ty) {
            return layer->getTileTypeAt({ tx, ty }) == TileType::SOLID;
            };

        // 获取图层边界
        const int layer_width = layer->getMapSize().x;
        const int layer_height = layer->getMapSize().y;

        // X sweep
        if (ds.x != 0.0f) {
            glm::vec2 new_pos = aabb_pos;
            new_pos.x += ds.x;

            const float top = new_pos.y + eps;
            const float bottom = new_pos.y + collider_size.y - eps;
            const int tile_y_top = static_cast<int>(std::floor((top - layer_offset.y) / tile_size.y));
            const int tile_y_bottom = static_cast<int>(std::floor((bottom - layer_offset.y) / tile_size.y));

            if (ds.x > 0.0f) {
                const float right = new_pos.x + collider_size.x - eps;
                const int tile_x = static_cast<int>(std::floor((right - layer_offset.x) / tile_size.x));
                if (tile_x >= 0 && tile_x < layer_width) {
                    bool hit = false;
                    if (tile_y_top >= 0 && tile_y_top < layer_height && isSolid(tile_x, tile_y_top)) hit = true;
                    if (tile_y_bottom >= 0 && tile_y_bottom < layer_height && isSolid(tile_x, tile_y_bottom)) hit = true;
                    if (hit) {
                        new_pos.x = layer_offset.x + static_cast<float>(tile_x) * tile_size.x - collider_size.x;
                        pc->velocity_.x = 0.0f;
                    }
                }
            } else {
                const float left = new_pos.x + eps;
                const int tile_x = static_cast<int>(std::floor((left - layer_offset.x) / tile_size.x));
                if (tile_x >= 0 && tile_x < layer_width) {
                    bool hit = false;
                    if (tile_y_top >= 0 && tile_y_top < layer_height && isSolid(tile_x, tile_y_top)) hit = true;
                    if (tile_y_bottom >= 0 && tile_y_bottom < layer_height && isSolid(tile_x, tile_y_bottom)) hit = true;
                    if (hit) {
                        new_pos.x = layer_offset.x + static_cast<float>(tile_x + 1) * tile_size.x;
                        pc->velocity_.x = 0.0f;
                    }
                }
            }
            aabb_pos.x = new_pos.x;
        }

        // Y sweep
        if (ds.y != 0.0f) {
            glm::vec2 new_pos = aabb_pos;
            new_pos.y += ds.y;

            const float left = new_pos.x + eps;
            const float right = new_pos.x + collider_size.x - eps;
            const int tile_x_left = static_cast<int>(std::floor((left - layer_offset.x) / tile_size.x));
            const int tile_x_right = static_cast<int>(std::floor((right - layer_offset.x) / tile_size.x));

            if (ds.y > 0.0f) {
                const float bottom = new_pos.y + collider_size.y - eps;
                const int tile_y = static_cast<int>(std::floor((bottom - layer_offset.y) / tile_size.y));
                if (tile_y >= 0 && tile_y < layer_height) {
                    bool hit = false;
                    if (tile_x_left >= 0 && tile_x_left < layer_width && isSolid(tile_x_left, tile_y)) hit = true;
                    if (tile_x_right >= 0 && tile_x_right < layer_width && isSolid(tile_x_right, tile_y)) hit = true;
                    if (hit) {
                        new_pos.y = layer_offset.y + static_cast<float>(tile_y) * tile_size.y - collider_size.y;
                        pc->velocity_.y = 0.0f;
                    }
                }
            } else {
                const float top = new_pos.y + eps;
                const int tile_y = static_cast<int>(std::floor((top - layer_offset.y) / tile_size.y));
                if (tile_y >= 0 && tile_y < layer_height) {
                    bool hit = false;
                    if (tile_x_left >= 0 && tile_x_left < layer_width && isSolid(tile_x_left, tile_y)) hit = true;
                    if (tile_x_right >= 0 && tile_x_right < layer_width && isSolid(tile_x_right, tile_y)) hit = true;
                    if (hit) {
                        new_pos.y = layer_offset.y + static_cast<float>(tile_y + 1) * tile_size.y;
                        pc->velocity_.y = 0.0f;
                    }
                }
            }
            aabb_pos.y = new_pos.y;
        }
    }

    // 计算新的变换位置
    // 变换位置 = AABB位置 - 碰撞器偏移量
    tc->setPosition(aabb_pos - collider_offset);

    // 应用速度限制
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
}

