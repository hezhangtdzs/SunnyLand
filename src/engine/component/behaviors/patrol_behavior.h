#pragma once
#include "../ai_behavior.h"

namespace engine::component {

    /**
     * @class PatrolBehavior
     * @brief 巡逻行为类，实现敌人在指定范围内左右移动
     */
    class PatrolBehavior final : public AIBehavior {
    private:
        float speed_; ///< 巡逻速度
        float patrolRange_; ///< 巡逻范围
        float startX_; ///< 起始X坐标
        bool movingRight_; ///< 是否向右移动
        
    public:
        /**
         * @brief 构造函数
         * 
         * @param speed 巡逻速度
         * @param patrolRange 巡逻范围
         */
        PatrolBehavior(float speed = 50.0f, float patrolRange = 100.0f);
        
        void init(engine::object::GameObject* owner) override;
        void update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context) override;
    };

}  // namespace engine::component
