#pragma once

#include "player_state.h"

namespace game::component::state
{
	/**
	 * @brief 玩家死亡状态。
	 * @details 死亡状态下禁止所有动作
	 */
	class DeadState final : public PlayerState {
	public:
		DeadState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~DeadState() override = default;

		// 死亡状态下禁止所有动作，使用基类默认实现（返回 nullptr）

	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}
