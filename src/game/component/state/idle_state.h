#pragma once

#include "player_state.h"

namespace game::component::state
{
	class IdleState final : public PlayerState {
	public:
		IdleState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~IdleState() override = default;
	protected:
		void enter() override;

		void exit() override;

		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;

		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}