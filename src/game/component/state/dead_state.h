#pragma once
#include "player_state.h"

namespace game::component::state {
	/**
	 * @brief 玩家死亡状态。
	 *
	 * 在此状态下，玩家将停止更新物理交互，播放死亡动画并准备从场景中移除。
	 */
	class DeadState : public PlayerState {
	public:
		explicit DeadState(game::component::PlayerComponent* player_component)
			: PlayerState(player_component) {}

	private:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}