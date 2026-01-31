#include "command_mapper.h"
#include "../../engine/core/context.h"
#include <spdlog/spdlog.h>

namespace game::command {

    void CommandMapper::bind(const std::string& action, std::unique_ptr<PlayerCommand> command) {
        if (command) {
            command_map_[action] = std::move(command);
        }
    }

    bool CommandMapper::execute(const std::string& action, engine::core::Context& context) {
        auto it = command_map_.find(action);
        if (it != command_map_.end() && it->second) {
            it->second->execute(context);
            return true;
        }
        return false;
    }

    bool CommandMapper::hasCommand(const std::string& action) const {
        return command_map_.find(action) != command_map_.end();
    }

    void CommandMapper::unbind(const std::string& action) {
        command_map_.erase(action);
    }

    void CommandMapper::clear() {
        command_map_.clear();
    }

    void CommandMapper::rebindPlayer(game::component::PlayerComponent* player) {
        // 注意：当前实现需要重新创建命令对象
        // 因为命令对象持有玩家指针
        // 在实际应用中，可以考虑使用更灵活的绑定机制
        spdlog::warn("CommandMapper::rebindPlayer 需要重新初始化命令映射");
        clear();
    }

} // namespace game::command
