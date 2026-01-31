#pragma once

#include "player_state.h"

namespace game::component::state
{
	/**
	 * @brief 玩家行走状态。
	 * @details 使用命令模式处理动作请求
	 */
	class WalkState final : public PlayerState {
	public:
		WalkState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~WalkState() override = default;

		// ========== 动作接口实现 ==========
		std::unique_ptr<PlayerState> moveLeft(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> moveRight(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> jump(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> climbUp(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> climbDown(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> stopMove(engine::core::Context& context) override;

	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}
