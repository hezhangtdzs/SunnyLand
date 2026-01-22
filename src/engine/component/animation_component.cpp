#include "animation_component.h"
#include "../render/animation.h"
#include "sprite_component.h"
#include "../object/game_object.h"
#include <spdlog/spdlog.h>
engine::component::AnimationComponent::~AnimationComponent() = default;

void engine::component::AnimationComponent::addAnimation(std::unique_ptr<engine::render::Animation> animation)
{
	if (animation) {
		const std::string& name = animation->getName();
		animations_[name] = std::move(animation);
	}
}

void engine::component::AnimationComponent::playAnimation(const std::string& name)
{
	auto it = animations_.find(name);
	if (it == animations_.end() || !it->second) {
		spdlog::warn("未找到 GameObject '{}' 的动画 '{}'", name, owner_ ? owner_->getName() : "未知");
		return;
	}

	// 如果已经在播放相同的动画，不重新开始（注释这一段则重新开始播放）
	if (current_animation_ == it->second.get() && is_playing_) {
		return;
	}

	current_animation_ = it->second.get();
	animation_timer_ = 0.0f;
	is_playing_ = true;

	// 尝试获取精灵组件（如果 init 尚未调用）
	if (!sprite_component_ && owner_) {
		sprite_component_ = owner_->getComponent<SpriteComponent>();
	}

	// 立即将精灵更新到第一帧
	if (sprite_component_ && !current_animation_->isEmpty()) {
		const auto& first_frame = current_animation_->getFrame(0.0f);
		SDL_FRect source_rect = {
			static_cast<float>(first_frame.src_rect.x),
			static_cast<float>(first_frame.src_rect.y),
			static_cast<float>(first_frame.src_rect.w),
			static_cast<float>(first_frame.src_rect.h)
		};
		sprite_component_->setSourceRect(source_rect);
		spdlog::debug("GameObject '{}' 开始播放动画 '{}'", owner_ ? owner_->getName() : "未知", name);
	}
}

std::string engine::component::AnimationComponent::getCurrentAnimationName() const
{
	if (current_animation_) {
		return current_animation_->getName();
	}
	return std::string();
}

bool engine::component::AnimationComponent::isAnimationFinished() const
{
	if (current_animation_) {
		return animation_timer_ >= current_animation_->getTotalDuration();
	}
	return false;
}

void engine::component::AnimationComponent::init()
{
	sprite_component_ = owner_->getComponent<SpriteComponent>();
	if (!sprite_component_) {
		spdlog::error("AnimationComponent 初始化失败：所属的 GameObject '{}' 缺少 SpriteComponent 组件。", owner_->getName());
	}
}

void engine::component::AnimationComponent::update(float delta_time, engine::core::Context&)
{
	if (!is_playing_ || !current_animation_ || !sprite_component_) return;

	animation_timer_ += delta_time;
	float total_duration = current_animation_->getTotalDuration();

	// 1. 处理动画结束逻辑
	if (animation_timer_ >= total_duration) {
		if (current_animation_->isLooping()) {
			// 使用 fmod 保留 delta_time 的余数，解决卡顿/漂移问题
			animation_timer_ = fmod(animation_timer_, total_duration);
		} else {
			// 非循环动画：锁定在结束位置，停止播放
			animation_timer_ = total_duration;
			is_playing_ = false;
		}
	}

	// 2. 获取并设置正确的帧（此时 timer 已经被处理好，不会导致刷新第一帧）
	const auto& frame = current_animation_->getFrame(animation_timer_);

	// 统一转换为 SDL_FRect
	SDL_FRect source_rect = {
		static_cast<float>(frame.src_rect.x),
		static_cast<float>(frame.src_rect.y),
		static_cast<float>(frame.src_rect.w),
		static_cast<float>(frame.src_rect.h)
	};
	sprite_component_->setSourceRect(source_rect);

	// 3. 处理一键移除逻辑（如爆炸特效）
	if (!is_playing_ && is_one_shot_removal_) {
		owner_->setNeedRemove(true);
	}
}
