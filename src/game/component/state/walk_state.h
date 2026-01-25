#pragma once
#include "player_state.h"
#include <memory>

namespace game::component::state {
	/**
	 * @brief 玩家行走状态。
	 */
	class WalkState final : public PlayerState {
	public:
		WalkState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~WalkState() override = default;
	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}