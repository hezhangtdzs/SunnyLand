#include "player_state.h"
#include "../player_component.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/object/game_object.h"
#include <spdlog/spdlog.h>

namespace game::component::state {

    void PlayerState::playAnimation(const std::string& animation_name) {
        if (!player_component_) {
            return;
        }

        auto animation_component = player_component_->getAnimationComponent();
        if (!animation_component) {
            return;
        }

        animation_component->playAnimation(animation_name);
    }

} // namespace game::component::state