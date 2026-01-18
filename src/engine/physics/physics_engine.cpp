#include "physics_engine.h"
#include <spdlog/spdlog.h>
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../component/collider_component.h"
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

void engine::physics::PhysicsEngine::unregisterPhysicsComponent(component::PhysicsComponent* physics_component)
{
	physics_components_.erase(std::remove(physics_components_.begin(), physics_components_.end(), physics_component), physics_components_.end());
	spdlog::info("物理组件注册注销 {}", static_cast<void*>(physics_component));
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

        // 更新位置：S += v * dt
        auto* tc = pc->getTransform();
        if (tc) {
			const glm::vec2 delta_pos = pc->velocity_ * dt;
			if (std::isfinite(delta_pos.x) && std::isfinite(delta_pos.y)) {
				tc->translate(delta_pos);
			}
        }
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
