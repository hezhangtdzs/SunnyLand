#include "player_commands.h"
#include "../component/player_component.h"
#include "../../engine/core/context.h"

namespace game::command {

    void MoveLeftCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->moveLeft(context);
        }
    }

    void MoveRightCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->moveRight(context);
        }
    }

    void JumpCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->jump(context);
        }
    }

    void AttackCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->attack(context);
        }
    }

    void ClimbUpCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->climbUp(context);
        }
    }

    void ClimbDownCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->climbDown(context);
        }
    }

    void StopMoveCommand::execute(engine::core::Context& context) {
        if (player_) {
            player_->stopMove(context);
        }
    }

} // namespace game::command
