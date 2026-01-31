#pragma once

#include "player_state.h"

namespace game::component::state
{
	/**
	 * @brief 玩家跳跃状态。
	 * @details 使用命令模式处理动作请求
	 */
	class JumpState final : public PlayerState {
	public:
		JumpState(PlayerComponent* player_component)
			: PlayerState(player_component) {}
		~JumpState() override = default;

		// ========== 动作接口实现 ==========
		std::unique_ptr<PlayerState> moveLeft(engine::core::Context& context) override;
		std::unique_ptr<PlayerState> moveRight(engine::core::Context& context) override;

	protected:
		void enter() override;
		void exit() override;
		std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) override;
	};
}
