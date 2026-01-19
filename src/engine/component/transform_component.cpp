#include "transform_component.h"
#include "component.h"
#include <glm/glm.hpp>
#include "../object/game_object.h"
#include "sprite_component.h"

// Keep collider offsets consistent with scaling changes
#include "collider_component.h"
void engine::component::TransformComponent::setScale(const glm::vec2& scale)
{
    scale_ = scale;
    if (owner_) {
        auto sprite_comp = owner_->getComponent<SpriteComponent>();
        if (sprite_comp) {
            sprite_comp->updateOffset();
        }
		auto collider_comp = owner_->getComponent<ColliderComponent>();
		if (collider_comp) {
			collider_comp->setAlignment(collider_comp->getAlignment());
		}
    }
}

void engine::component::TransformComponent::translate(const glm::vec2& offset)
{
	position_ += offset;
}
