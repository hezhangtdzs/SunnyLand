#include "patrol_behavior.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/object/game_object.h"

game::component::PatrolBehavior::PatrolBehavior(float speed, float patrolRange)
    : speed_(speed),
      patrolRange_(patrolRange),
      startX_(0.0f),
      movingRight_(true)
{
}

void game::component::PatrolBehavior::init(engine::object::GameObject* owner)
{
    if (owner) {
        auto* transform = owner->getComponent<engine::component::TransformComponent>();
        if (transform) {
            startX_ = transform->getPosition().x;
        }
    }
}

void game::component::PatrolBehavior::update(engine::object::GameObject* owner, float /*deltaTime*/, engine::core::Context& /*context*/)
{
    if (!owner) return;

    auto* transform = owner->getComponent<engine::component::TransformComponent>();
    auto* physics = owner->getComponent<engine::component::PhysicsComponent>();
    auto* sprite = owner->getComponent<engine::component::SpriteComponent>();

    if (!transform || !physics) return;

    auto position = transform->getPosition();

    // 计算移动方向
    if (movingRight_) {
        // 如果达到巡逻右边界，或者碰到右侧障碍物，则转向
        if (position.x - startX_ >= patrolRange_ || physics->hasCollidedRight()) {
            movingRight_ = false;
        }
    } else {
        // 如果达到巡逻左边界，或者碰到左侧障碍物，则转向
        if (startX_ - position.x >= patrolRange_ || physics->hasCollidedLeft()) {
            movingRight_ = true;
        }
    }

    // 设置速度
    physics->velocity_.x = movingRight_ ? speed_ : -speed_;

    // 播放行走动画
    if (auto* anim = owner->getComponent<engine::component::AnimationComponent>()) {
        anim->playAnimation("walk");
    }

    // 设置精灵翻转 (假设默认图片向左：则向右移动需要翻转)
    if (sprite) {
        sprite->setFlipped(movingRight_);
    }
}
