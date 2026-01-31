#pragma once

#include "player_state.h"

namespace game::component::state
{
	/**
	 * @brief 玩家待机状态。
	 * @details 使用命令模式处理动作请求
	 */
	class IdleState final : public PlayerState {
	public:
		IdleState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~IdleState() override = default;

		// ========== 动作接口实现 ==========
		std::unique_ptr<PlayerState> moveLeft(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> moveRight(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> jump(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> climbUp(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> climbDown(engine::core::Context& context) override;

	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}
