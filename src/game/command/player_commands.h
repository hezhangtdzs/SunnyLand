#pragma once
#include <memory>

namespace engine::core {
    class Context;
}

namespace game::component {
    class PlayerComponent;
}

namespace game::command {

    /**
     * @brief 玩家命令基类
     * @details 命令模式的核心接口，封装对玩家的操作请求
     */
    class PlayerCommand {
    public:
        virtual ~PlayerCommand() = default;
        
        /**
         * @brief 执行命令
         * @param context 引擎上下文
         */
        virtual void execute(engine::core::Context& context) = 0;
        
        /**
         * @brief 撤销命令（可选）
         * @details 默认空实现，需要撤销功能的命令可重写
         */
        virtual void undo() {}
    };

    /**
     * @brief 向左移动命令
     */
    class MoveLeftCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit MoveLeftCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

    /**
     * @brief 向右移动命令
     */
    class MoveRightCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit MoveRightCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

    /**
     * @brief 跳跃命令
     */
    class JumpCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit JumpCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

    /**
     * @brief 攻击命令
     */
    class AttackCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit AttackCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

    /**
     * @brief 向上攀爬命令
     */
    class ClimbUpCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit ClimbUpCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

    /**
     * @brief 向下攀爬命令
     */
    class ClimbDownCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit ClimbDownCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

    /**
     * @brief 停止移动命令
     */
    class StopMoveCommand : public PlayerCommand {
        game::component::PlayerComponent* player_;
    public:
        explicit StopMoveCommand(game::component::PlayerComponent* player) : player_(player) {}
        void execute(engine::core::Context& context) override;
    };

} // namespace game::command
