#pragma once
#include "../../../engine/component/ai_behavior.h"

namespace game::component {

    /**
     * @class UpDownBehavior
     * @brief 飞行行为类，实现敌人在垂直方向上上下移动
     */
    class UpDownBehavior final : public engine::component::AIBehavior {
    private:
        float speed_; ///< 飞行速度
        float moveRange_; ///< 移动范围
        float startY_; ///< 起始Y坐标
        bool movingUp_; ///< 是否向上移动
        
    public:
        /**
         * @brief 构造函数
         * 
         * @param speed 飞行速度
         * @param moveRange 移动范围
         */
        UpDownBehavior(float speed = 40.0f, float moveRange = 80.0f);
        
        void init(engine::object::GameObject* owner) override;
        void update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context) override;
    };

}  // namespace game::component
