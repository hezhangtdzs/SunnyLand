#include "physics_component.h"
#include <spdlog/spdlog.h>
#include "../object/game_object.h"
#include "transform_component.h"
#include "../physics/physics_engine.h"
engine::component::PhysicsComponent::PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity, float mass)
	: physics_engine_(physics_engine), use_gravity_(use_gravity), mass_(mass > 0.0f ? mass : 1.0f)
{
	if (!physics_engine_) {
		spdlog::error("PhysicsEngine is null");
	}
	spdlog::trace("物理组件创建完成，质量: {}, 使用重力: {}", mass_, use_gravity_);
}

engine::component::PhysicsComponent::~PhysicsComponent() = default;

void engine::component::PhysicsComponent::init()
{
	if (!owner_) {
		spdlog::error("PhysicsComponent 初始化失败：所属对象为空");
		return;
	}
	transform_component_ = owner_->getComponent<TransformComponent>();
	if (!transform_component_) {
		spdlog::error("PhysicsComponent 初始化失败：所属对象缺少 TransformComponent 组件");
		return;
	}
	if (physics_engine_) {
		physics_engine_->registerPhysicsComponent(this);
		spdlog::trace("PhysicsComponent 初始化完成并注册到 PhysicsEngine");
	} else {
		spdlog::error("PhysicsComponent 初始化失败：PhysicsEngine 为空");
	}
}
void engine::component::PhysicsComponent::update(float /*deltaTime*/, engine::core::Context& /*context*/)
{
	// 物理更新由 PhysicsEngine 统一处理
}
void engine::component::PhysicsComponent::clean()
{
	if (physics_engine_) {
		physics_engine_->unregisterPhysicsComponent(this);
		spdlog::trace("PhysicsComponent 已从 PhysicsEngine 注销");
	}
}
