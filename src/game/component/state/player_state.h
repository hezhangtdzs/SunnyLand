#pragma once
#include "../../../engine/core/context.h"
#include <memory>
#include <string>

namespace game::component {
	class PlayerComponent;
}

namespace game::component::state {
	/**
	 * @brief 玩家状态基类，定义了状态机的基础接口。
	 * @details 使用命令模式解耦输入检测和动作执行
	 */
	class PlayerState {
		friend class game::component::PlayerComponent;
	protected:
		game::component::PlayerComponent* player_component_{ nullptr }; ///< 所属的玩家组件

	public:
		PlayerState(game::component::PlayerComponent* player_component)
			: player_component_(player_component) {}
		virtual ~PlayerState() = default;

		// 禁止拷贝和移动
		PlayerState(const PlayerState&) = delete;
		PlayerState& operator=(const PlayerState&) = delete;
		PlayerState(PlayerState&&) = delete;
		PlayerState& operator=(PlayerState&&) = delete;

		/**
		 * @brief 播放指定名称的动画（辅助方法）
		 * @param animation_name 动画名称
		 */
		void playAnimation(const std::string& animation_name);

		// ========== 动作接口（供PlayerComponent调用） ==========

		/**
		 * @brief 处理向左移动请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> moveLeft(engine::core::Context& context) { return nullptr; }

		/**
		 * @brief 处理向右移动请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> moveRight(engine::core::Context& context) { return nullptr; }

		/**
		 * @brief 处理跳跃请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> jump(engine::core::Context& context) { return nullptr; }

		/**
		 * @brief 处理攻击请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> attack(engine::core::Context& context) { return nullptr; }

		/**
		 * @brief 处理向上攀爬请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> climbUp(engine::core::Context& context) { return nullptr; }

		/**
		 * @brief 处理向下攀爬请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> climbDown(engine::core::Context& context) { return nullptr; }

		/**
		 * @brief 处理停止移动请求
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> stopMove(engine::core::Context& context) { return nullptr; }

	protected:
		virtual void enter() = 0; ///< 进入该状态时的初始化逻辑
		virtual void exit() = 0;  ///< 退出该状态时的清理逻辑

		/**
		 * @brief 更新状态逻辑
		 * @param delta_time 帧时间
		 * @param context 引擎上下文
		 * @return 返回新状态指针，若不发生跳转则返回 nullptr
		 */
		virtual std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context& context) = 0;
	};
}
