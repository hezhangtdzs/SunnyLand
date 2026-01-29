#include "player_state.h"
#include "../player_component.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/object/game_object.h"
#include <spdlog/spdlog.h>

namespace game::component::state {

/**
 * @brief 播放指定名称的动画
 * 
 * @param animation_name 动画名称
 * @details 查找玩家组件的动画组件并播放指定动画
 */
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