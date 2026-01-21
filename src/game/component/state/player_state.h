#pragma once
#include "../../../engine/core/context.h"
#include <memory>

namespace game::component {
	class PlayerComponent;
}

namespace game::component::state {
	class PlayerState {
		friend class game::component::PlayerComponent;
	protected:
		game::component::PlayerComponent* player_component_{ nullptr };
	public:
		PlayerState(game::component::PlayerComponent* player_component)
			: player_component_(player_component) {}
		virtual ~PlayerState() = default;
		// 禁止拷贝和移动构造
		PlayerState(const PlayerState&) = delete;
		PlayerState& operator=(const PlayerState&) = delete;
		PlayerState(PlayerState&&) = delete;
		PlayerState& operator=(PlayerState&&) = delete;
	protected:

		virtual void enter() = 0;
		virtual void exit() = 0;
		virtual std::unique_ptr<PlayerState> handleInput(engine::core::Context& context) = 0;
		virtual std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) = 0;
	};
}