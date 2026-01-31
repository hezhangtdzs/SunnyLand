#include "dead_state.h"
#include "../player_component.h"
#include "../../../engine/object/game_object.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/physics_component.h"
#include <spdlog/spdlog.h>

namespace game::component::state {

void DeadState::enter()
{
    spdlog::debug("玩家进入死亡状态。");
    playAnimation("hurt");  // 播放死亡(受伤)动画

    // 应用击退力（只向上）
    auto physics_component = player_component_->getPhysicsComponent();
    physics_component->velocity_ = glm::vec2(0.0f, -200.0f);  // 向上弹起

    // 禁用碰撞(使其可以掉出屏幕)
    auto collider_component = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>();
    if (collider_component) {
        collider_component->setIsActive(false);
    }
}

void DeadState::exit()
{
}

std::unique_ptr<PlayerState> DeadState::update(float delta_time, engine::core::Context& context)
{
	// 死亡状态不处理任何逻辑，等待场景切换或重生
	return nullptr;
}

} // namespace game::component::state
