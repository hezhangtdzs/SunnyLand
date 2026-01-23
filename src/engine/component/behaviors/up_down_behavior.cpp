#include "up_down_behavior.h"
#include "../transform_component.h"
#include "../physics_component.h"
#include "../animation_component.h"
#include "../../object/game_object.h"

engine::component::UpDownBehavior::UpDownBehavior(float speed, float moveRange)
    : speed_(speed),
      moveRange_(moveRange),
      startY_(0.0f),
      movingUp_(true)
{
}

void engine::component::UpDownBehavior::init(engine::object::GameObject* owner)
{
    if (owner) {
        auto* transform = owner->getComponent<TransformComponent>();
        if (transform) {
            startY_ = transform->getPosition().y;
        }
        auto* physics = owner->getComponent<PhysicsComponent>();
        if (physics) {
            physics->setUseGravity(false);
        }
    }
}

void engine::component::UpDownBehavior::update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context)
{
    if (!owner) return;
    
    auto* transform = owner->getComponent<TransformComponent>();
    auto* physics = owner->getComponent<PhysicsComponent>();
    
    if (!transform || !physics) return;
    
    auto position = transform->getPosition();
    
    // 计算移动方向
    if (movingUp_) {
        // 如果达到上边界，或者碰到上方障碍物，则转向
        if (startY_ - position.y >= moveRange_ || physics->hasCollidedAbove()) {
            movingUp_ = false;
        }
    } else {
        // 如果达到下边界，或者碰到下方障碍物，则转向
        if (position.y - startY_ >= moveRange_ || physics->hasCollidedBelow()) {
            movingUp_ = true;
        }
    }
    
    // 设置速度
    physics->velocity_.y = movingUp_ ? -speed_ : speed_;

    // 播放飞行动画
    if (auto* anim = owner->getComponent<AnimationComponent>()) {
        anim->playAnimation("fly");
    }
}
