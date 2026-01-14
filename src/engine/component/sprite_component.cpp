#include "sprite_component.h"
#include "transform_component.h"
#include "../object/game_object.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "../render/camera.h"
#include <stdexcept>          // 用于 std::runtime_error
#include <spdlog/spdlog.h>

namespace engine::component {
    SpriteComponent::SpriteComponent(
                const std::string& texture_id,
                engine::resource::ResourceManager& resource_manager,
                engine::utils::Alignment alignment,
                std::optional<SDL_FRect> source_rect_opt,
                bool is_flipped) 
                : resource_manager_(&resource_manager), 
                sprite_(texture_id, source_rect_opt, is_flipped),
                alignment_(alignment)
{  
    if (!resource_manager_) {
        spdlog::critical("创建 SpriteComponent 时 ResourceManager 为空！，此组件将无效。");
        // 不要在游戏主循环中使用 try...catch / throw，会极大影响性能
    }
    // offset_ 和 sprite_size_ 将在 init 中计算
    spdlog::trace("创建 SpriteComponent，纹理ID: {}", texture_id);
}

    void SpriteComponent::updateOffset()
    {
        if(sprite_size_ == glm::vec2{0.0f, 0.0f}) {
            spdlog::warn("SpriteComponent 更新偏移量警告：精灵尺寸为零，无法计算偏移量。");
            return;
        }
        auto scale =transform_-> getScale();
        switch(alignment_){
            case engine::utils::Alignment::TOP_LEFT:      offset_ = glm::vec2{0.0f, 0.0f} * scale; break;
            case engine::utils::Alignment::TOP_CENTER:    offset_ = glm::vec2{-sprite_size_.x / 2.0f, 0.0f} * scale; break;
            case engine::utils::Alignment::TOP_RIGHT:     offset_ = glm::vec2{-sprite_size_.x, 0.0f} * scale; break;
            case engine::utils::Alignment::CENTER_LEFT:   offset_ = glm::vec2{0.0f, -sprite_size_.y / 2.0f} * scale; break;
            case engine::utils::Alignment::CENTER:        offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y / 2.0f} * scale; break;
            case engine::utils::Alignment::CENTER_RIGHT:  offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y / 2.0f} * scale; break;
            case engine::utils::Alignment::BOTTOM_LEFT:   offset_ = glm::vec2{0.0f, -sprite_size_.y} * scale; break;
            case engine::utils::Alignment::BOTTOM_CENTER: offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y} * scale; break;
            case engine::utils::Alignment::BOTTOM_RIGHT:  offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y} * scale; break;
            case engine::utils::Alignment::NONE:
            default:                                      break;
        }
    }

    void SpriteComponent::setSpriteById(const std::string &texture_id, const std::optional<SDL_FRect> &source_rect_opt)
    {
        sprite_.setTextureId(texture_id);
        sprite_.setSourceRect(source_rect_opt);
        updateSpriteSize();
        updateOffset();
    }

    void SpriteComponent::setSourceRect(const std::optional<SDL_FRect> &source_rect_opt)
    {
        sprite_.setSourceRect(source_rect_opt);
        updateSpriteSize();
        updateOffset();
    }

    void SpriteComponent::setAlignment(engine::utils::Alignment anchor)
    {
        alignment_ = anchor;
        updateOffset();
    }

    void SpriteComponent::updateSpriteSize()
    {
        if(!resource_manager_) {
            spdlog::critical("SpriteComponent 更新精灵尺寸失败：ResourceManager 为空！");
            return;
        }
        if(sprite_.getSourceRect().has_value()) {
            const SDL_FRect& src_rect = sprite_.getSourceRect().value();
            sprite_size_.x = src_rect.w;
            sprite_size_.y = src_rect.h;
        } else {
            sprite_size_ = resource_manager_->getTextureSize(sprite_.getTextureId());
        }
    }

    void SpriteComponent::init()
    {
        if (!owner_) {
            spdlog::critical("SpriteComponent 初始化失败：所属的 GameObject 为空！");
            return;
        }
        transform_ = owner_->getComponent<TransformComponent>();
        if (!transform_) {
            spdlog::warn("SpriteComponent 初始化警告：所属的 GameObject '{}' 没有 TransformComponent，位置和对齐可能无法正确计算。", owner_->getName());
        }
        updateSpriteSize();
        updateOffset();
    }

    void SpriteComponent::update(float /*deltaTime*/, engine::core::Context &/*context*/)
    {
    }

    void SpriteComponent::render(engine::core::Context &context)
    {
        if(is_hidden_ || !transform_ || !resource_manager_) {
            return;
        }
        const glm::vec2& position = transform_->getPosition() + offset_;
        const glm::vec2& scale = transform_->getScale();
        float rotation = transform_->getRotation();
        context.getRenderer().drawSprite(context.getCamera(),sprite_, position, scale, rotation);
    }

} // namespace engine::component