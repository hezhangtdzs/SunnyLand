#pragma once

#include "player_state.h"

namespace game::component::state
{
	/**
	 * @brief 玩家受伤状态。
	 * @details 受伤状态下禁止所有动作
	 */
	class HurtState final : public PlayerState {
	public:
		HurtState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~HurtState() override = default;

		// 受伤状态下禁止所有动作，使用基类默认实现（返回 nullptr）

	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;

	private:
		float stunned_timer_ = 0.0f; ///< 僵直计时器
	};
}
