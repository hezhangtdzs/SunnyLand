#include "collider_component.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include "../physics/collider.h"
#include "../object/game_object.h"
#include "transform_component.h"
engine::component::ColliderComponent::ColliderComponent(std::unique_ptr<engine::physics::Collider> collider,
	engine::utils::Alignment alignment, bool is_trigger, bool is_active)
	: collider_(std::move(collider)),
	  alignment_(alignment),
	  is_trigger_(is_trigger),
	is_active_(is_active)
{
	if (!collider_) {
		spdlog::error("ColliderComponent 创建失败：传入的 Collider 指针为空");
	} else {
		spdlog::trace("ColliderComponent 创建完成，类型: {}", static_cast<int>(collider_->getType()));
	}
}

engine::utils::Rect engine::component::ColliderComponent::getWorldAABB() const
{
	if (!transform_component_ || !collider_) {
		spdlog::error("ColliderComponent 获取世界AABB失败：缺少 TransformComponent 或 Collider");
		return engine::utils::Rect{{0,0},{0,0}};
	}
	auto top_left = transform_component_->getPosition() + offset_;
	auto scale = transform_component_->getScale();
	auto size = collider_->getAABBSize()*scale;
	return engine::utils::Rect{top_left, size};
}

void engine::component::ColliderComponent::setAlignment(engine::utils::Alignment alignment)
{
	alignment_ = alignment;
	if(transform_component_&&collider_){
		updateOffset();
	}
}

void engine::component::ColliderComponent::init()
{
	if(owner_) {
		transform_component_ = owner_->getComponent<TransformComponent>();
		if (!transform_component_) {
			spdlog::error("ColliderComponent 初始化失败：所属对象缺少 TransformComponent 组件");
		} else {
			spdlog::trace("ColliderComponent 初始化完成");
		}
	} else {
		spdlog::error("ColliderComponent 初始化失败：所属对象为空");
	}
	updateOffset();
}

void engine::component::ColliderComponent::updateOffset()
{
	if (!transform_component_ || !collider_) {
		return;
	}
	glm::vec2 position = transform_component_->getPosition();
	glm::vec2 collider_size = collider_->getAABBSize();
	auto scale = transform_component_->getScale();
	// 根据对齐方式调整偏移量
	switch (alignment_) {
	case engine::utils::Alignment::TOP_LEFT:
		offset_ = { 0, 0 };
		break;
	case engine::utils::Alignment::TOP_CENTER:
		offset_ = glm::vec2{ -collider_size.x / 2.0f, 0 } * scale;
		break;
	case engine::utils::Alignment::TOP_RIGHT:
		offset_ = glm::vec2{ -collider_size.x, 0 } * scale;
		break;
	case engine::utils::Alignment::CENTER_LEFT:
		offset_ = glm::vec2{ 0, -collider_size.y / 2.0f } * scale;
		break;
	case engine::utils::Alignment::CENTER:
		offset_ = glm::vec2{ -collider_size.x / 2.0f, -collider_size.y / 2.0f } * scale;
		break;
	case engine::utils::Alignment::CENTER_RIGHT:
		offset_ = glm::vec2{ -collider_size.x, -collider_size.y / 2.0f } * scale;
		break;
	case engine::utils::Alignment::BOTTOM_LEFT:
		offset_ = glm::vec2{ 0, -collider_size.y } * scale;
		break;
	case engine::utils::Alignment::BOTTOM_CENTER:
		offset_ = glm::vec2{ -collider_size.x / 2.0f, -collider_size.y } * scale;
		break;
	case engine::utils::Alignment::BOTTOM_RIGHT:
		offset_ = glm::vec2{ -collider_size.x, -collider_size.y } * scale;
		break;
	default:
		break;
	}
	
}


