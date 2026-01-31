#pragma once
#include "../../../engine/component/ai_behavior.h"

namespace game::component {

    /**
     * @class JumpBehavior
     * @brief 跳跃行为类，实现敌人的跳跃和地面移动
     */
    class JumpBehavior final : public engine::component::AIBehavior {
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
        
        /**
         * @brief 初始化行为
         * 
         * @param owner 拥有该行为的游戏对象
         */
        void init(engine::object::GameObject* /*owner*/) override;

        /**
         * @brief 更新行为
         *
         * @param owner 拥有该行为的游戏对象
         * @param deltaTime 自上次更新以来的时间间隔
         * @param context 游戏上下文
         */
        void update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context) override;
    };

}  // namespace game::component
