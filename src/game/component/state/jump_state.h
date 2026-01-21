#pragma once
#include "player_state.h"
#include <memory>

namespace game::component::state {
	class JumpState final : public PlayerState {
	public:
		JumpState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~JumpState() override = default;
	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}