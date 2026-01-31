#pragma once
#include "player_commands.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace engine::core {
    class Context;
}

namespace game::component {
    class PlayerComponent;
}

namespace game::command {

    /**
     * @brief 命令映射器
     * @details 管理输入动作到命令对象的映射，实现输入与动作的解耦
     */
    class CommandMapper {
    public:
        CommandMapper() = default;
        ~CommandMapper() = default;

        // 禁止拷贝，允许移动
        CommandMapper(const CommandMapper&) = delete;
        CommandMapper& operator=(const CommandMapper&) = delete;
        CommandMapper(CommandMapper&&) = default;
        CommandMapper& operator=(CommandMapper&&) = default;

        /**
         * @brief 绑定动作到命令
         * @param action 动作名称（如 "move_left"）
         * @param command 命令对象
         */
        void bind(const std::string& action, std::unique_ptr<PlayerCommand> command);

        /**
         * @brief 执行指定动作的命令
         * @param action 动作名称
         * @param context 引擎上下文
         * @return 是否成功执行（命令存在且执行成功）
         */
        bool execute(const std::string& action, engine::core::Context& context);

        /**
         * @brief 检查动作是否已绑定
         * @param action 动作名称
         * @return 是否已绑定
         */
        bool hasCommand(const std::string& action) const;

        /**
         * @brief 解除动作绑定
         * @param action 动作名称
         */
        void unbind(const std::string& action);

        /**
         * @brief 清除所有绑定
         */
        void clear();

        /**
         * @brief 重新绑定玩家（用于切换控制目标）
         * @param player 新的玩家组件
         * @details 双人控制时使用，将所有命令重新绑定到新玩家
         */
        void rebindPlayer(game::component::PlayerComponent* player);

    private:
        std::unordered_map<std::string, std::unique_ptr<PlayerCommand>> command_map_;
    };

} // namespace game::command
