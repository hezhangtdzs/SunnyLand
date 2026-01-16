#include "parallax_component.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>
#include "../object/game_object.h"
#include "../core/context.h"
#include "../render/renderer.h"

engine::component::ParallaxComponent::ParallaxComponent(std::string& texture_id, const glm::vec2& parallax_factor, const glm::bvec2& repeat) :
	sprite_(engine::render::Sprite(texture_id)),
	parallax_factor_(parallax_factor),
	repeat_(repeat)
{
	spdlog::trace("ParallaxComponent 构造完成，纹理ID：{}", texture_id);
}
engine::component::ParallaxComponent::~ParallaxComponent() = default;

void engine::component::ParallaxComponent::init()
{
	if (!owner_) {
		spdlog::error("ParallaxComponent 初始化失败：所属对象为空");
		return;
	}
	transform_ = owner_->getComponent<TransformComponent>();
	if(!transform_){
		spdlog::error("ParallaxComponent 初始化失败：所属对象缺少 TransformComponent 组件");
	}
	else{
		spdlog::trace("ParallaxComponent 初始化完成");
	}
}
void engine::component::ParallaxComponent::update(float /*deltaTime*/, engine::core::Context& /*context*/)
{
	// 视差背景通常不需要在 update 中做任何事情
}
void engine::component::ParallaxComponent::render(engine::core::Context& context)
{
	if (is_hidden_ || !transform_) {
		return;
	}
	context.getRenderer().drawParallax(context.getCamera(),
									   sprite_,transform_->getPosition(), 
									   parallax_factor_,repeat_,
									   transform_->getScale());
}

void engine::component::ParallaxComponent::handleInput(engine::core::Context& /*context*/)
{
	// 视差组件通常不处理输入
}

void engine::component::ParallaxComponent::clean()
{
	// 视差组件没有需要手动释放的动态资源
}

