#include "jump_behavior.h"
#include "../transform_component.h"
#include "../physics_component.h"
#include "../sprite_component.h"
#include "../animation_component.h"
#include "../audio_component.h"
#include "../../object/game_object.h"

engine::component::JumpBehavior::JumpBehavior(float xMin, float xMax, float moveSpeed, float jumpForce, float jumpCooldown)
    : moveSpeed_(moveSpeed),
      jumpForce_(jumpForce),
      xMin_(xMin),
      xMax_(xMax),
      movingRight_(true),
      jumpCooldown_(jumpCooldown),
      jumpTimer_(0.0f)
{
}

void engine::component::JumpBehavior::init(engine::object::GameObject* /*owner*/)
{
}

void engine::component::JumpBehavior::update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context)
{
    if (!owner) return;
    
    auto* transform = owner->getComponent<TransformComponent>();
    auto* physics = owner->getComponent<PhysicsComponent>();
    auto* sprite = owner->getComponent<SpriteComponent>();
    auto* anim = owner->getComponent<AnimationComponent>();
    
    if (!transform || !physics) return;
    
    bool onGround = physics->hasCollidedBelow();
	if (onGround && !wasOnGround_) {
		if (auto* audio = owner->getComponent<AudioComponent>()) {
			audio->playSoundNearCamera("cry", context, 360.0f);
		}
	}
	wasOnGround_ = onGround;
    
    if (onGround) {
        // 落地后立即停止水平移动，解决滑动问题
        physics->velocity_.x = 0.0f;

        // 处理跳跃计时器
        jumpTimer_ += deltaTime;

        // 在冷却时间的一半时执行转向逻辑，实现“落地后先不动，过一会儿再转向”
        // 也可以设置为 jumpTimer_ >= jumpCooldown_ - 0.2f 等起跳前一瞬间转向
        if (jumpTimer_ >= jumpCooldown_ * 0.5f) {
            auto position = transform->getPosition();
            if (movingRight_) {
                if (position.x >= xMax_ || physics->hasCollidedRight()) {
                    movingRight_ = false;
                }
            } else {
                if (position.x <= xMin_ || physics->hasCollidedLeft()) {
                    movingRight_ = true;
                }
            }
        }

        // 执行跳跃：仅在冷却完成后
        if (jumpTimer_ >= jumpCooldown_) {
            physics->velocity_.y = -jumpForce_;
            // 跳跃瞬间赋予水平速度
            physics->velocity_.x = movingRight_ ? moveSpeed_ : -moveSpeed_;
            jumpTimer_ = 0.0f; // 重置计时器
        }

        // 地面动画
        if (anim) anim->playAnimation("idle");
    } else {
        // 空中动画切换
        if (anim) {
            if (physics->velocity_.y < 0) {
                anim->playAnimation("jump");
            } else {
                anim->playAnimation("fall");
            }
        }
    }
    
    // 设置精灵翻转
    if (sprite) {
        sprite->setFlipped(movingRight_);
    }
}
