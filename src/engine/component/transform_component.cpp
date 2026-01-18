#include "transform_component.h"
#include "component.h"
#include <glm/glm.hpp>
#include "../object/game_object.h"
#include "sprite_component.h"
void engine::component::TransformComponent::setScale(const glm::vec2& scale)
{
    scale_ = scale;
    if (owner_) {
        auto sprite_comp = owner_->getComponent<SpriteComponent>();
        if (sprite_comp) {
            sprite_comp->updateOffset();
        }
    }
}

void engine::component::TransformComponent::translate(const glm::vec2& offset)
{
	position_ += offset;
}
