#pragma once
#include "../ai_behavior.h"

namespace engine::component {

    /**
     * @class JumpBehavior
     * @brief 跳跃行为类，实现敌人的跳跃和地面移动
     */
    class JumpBehavior final : public AIBehavior {
    private:
        float moveSpeed_; ///< 移动速度
        float jumpForce_; ///< 跳跃力度
        float xMin_; ///< 最小X坐标
        float xMax_; ///< 最大X坐标
        bool movingRight_; ///< 是否向右移动
        float jumpCooldown_; ///< 跳跃冷却时间
        float jumpTimer_; ///< 跳跃计时器
        bool wasOnGround_{ false }; ///< 是否在地面上
        
    public:
        /**
         * @brief 构造函数
         * 
         * @param xMin 最小X坐标
         * @param xMax 最大X坐标
         * @param moveSpeed 移动速度
         * @param jumpForce 跳跃力度
         * @param jumpCooldown 跳跃冷却时间
         */
        JumpBehavior(float xMin, float xMax, float moveSpeed = 60.0f, float jumpForce = 250.0f, float jumpCooldown = 2.0f);
        
        void init(engine::object::GameObject* owner) override;
        void update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context) override;
    };

}  // namespace engine::component
