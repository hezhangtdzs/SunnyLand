#pragma once

#include "player_state.h"

namespace game::component::state
{
	/**
	 * @brief 玩家攀爬状态。
	 */
	class ClimbState final : public PlayerState {
	public:
		ClimbState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~ClimbState() override = default;
	protected:
		void enter() override;
		void exit() override;

		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;

	private:
		float climb_speed_{ 120.0f }; ///< 攀爬移动速度
	};
}
