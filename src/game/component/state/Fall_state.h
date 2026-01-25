#pragma once
#include "player_state.h"
#include <memory>

namespace game::component::state {
	/**
	 * @brief 玩家空中下落状态。
	 */
	class FallState final : public PlayerState {
	public:
		FallState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~FallState() override = default;
	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}