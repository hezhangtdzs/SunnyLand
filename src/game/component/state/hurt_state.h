#pragma once
#include "player_state.h"

namespace game::component::state {
	/**
	 * @brief 玩家受伤状态。
	 *
	 * 在此状态下，玩家会播放受伤动画，并在一段时间内无法进行其他操作。
	 * 受伤状态结束后，玩家将返回到待机状态。
	 */
	class HurtState : public PlayerState {
	public:
		explicit HurtState(game::component::PlayerComponent* player_component)
			: PlayerState(player_component) {}
	private:
		void enter() override;
		void exit() override;

		float stunned_timer_ = 0.0f;

		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}