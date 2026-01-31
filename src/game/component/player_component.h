#pragma once
#include "../../engine/object/game_object.h"
#include <memory>
#include "state/player_state.h"

namespace engine::component {
	class TransformComponent;
	class SpriteComponent;
	class PhysicsComponent;
	class AnimationComponent;
	class HealthComponent;
	class AudioComponent;
}

namespace game::component
{
	namespace state
	{
		class PlayerState;
	}
	/**
	 * @brief 玩家组件，负责玩家的状态管理、物理交互和动画同步。
	 * @details 使用命令模式解耦输入处理和动作执行
	 */
	class PlayerComponent : public engine::component::Component {
		friend class engine::object::GameObject;
	private:
		engine::component::TransformComponent* transform_component_{ nullptr }; ///< 变换组件
		engine::component::SpriteComponent* sprite_component_{ nullptr };       ///< 精灵渲染组件
		engine::component::PhysicsComponent* physics_component_{ nullptr };     ///< 物理组件
		engine::component::AnimationComponent* animation_component_ = nullptr;   ///< 动画组件
		engine::component::HealthComponent* health_component_ = nullptr;         ///< 生命值组件
		engine::component::AudioComponent* audio_component_ = nullptr;         ///< 音频组件

		std::unique_ptr<state::PlayerState> current_state_{ nullptr };         ///< 当前玩家状态
		bool is_dead_{ false };                                               ///< 玩家是否死亡

		float move_force_{ 200.0f };                                       ///< 移动推力
		float jump_force_{ 350.0f };                                      ///< 跳跃力
		float max_move_speed_{ 150.0f };                                  ///< 最大移动速度
		float friction_{ 0.8f };                                          ///< 地面摩擦力
		float stunned_duration_ = 0.4f;                                   ///< 受伤僵直时长
		float coyote_timer_ = 0.0f;                                       ///< 土狼时间计时器
		float coyote_grace_duration_ = 0.12f;                                 ///< 土狼时间宽限时长

	public:
		PlayerComponent();
		~PlayerComponent() override = default;
		PlayerComponent(const PlayerComponent&) = delete;
		PlayerComponent& operator=(const PlayerComponent&) = delete;
		PlayerComponent(PlayerComponent&&) = delete;
		PlayerComponent& operator=(PlayerComponent&&) = delete;

		engine::component::TransformComponent* getTransformComponent() const { return transform_component_; }
		engine::component::SpriteComponent* getSpriteComponent() const { return sprite_component_; }
		engine::component::PhysicsComponent* getPhysicsComponent() const { return physics_component_; }
		engine::component::AnimationComponent* getAnimationComponent() const;
		engine::component::HealthComponent* getHealthComponent() const { return health_component_; }
		engine::component::AudioComponent* getAudioComponent() const { return audio_component_; }

		/**
		 * @brief 玩家受攻击
		 * @param damage 伤害值
		 * @param context 引擎上下文
		 * @return 是否成功扣血
		 */
		bool takeDamage(int damage, engine::core::Context& context);

		/**
		 * @brief 检查玩家是否处于梯子范围内
		 * @param context 引擎上下文
		 * @return 是否在梯子上
		 */
		bool isOverLadder(engine::core::Context& context) const;

		bool isDead() const { return is_dead_; }
		void setDead(bool is_dead) { is_dead_ = is_dead; }
		float getMoveForce() const { return move_force_; }
		float getJumpForce() const { return jump_force_; }
		float getMaxMoveSpeed() const { return max_move_speed_; }
		float getFriction() const { return friction_; }
		float getStunnedDuration() const { return stunned_duration_; }
		void setCoyoteTimer(float time) { coyote_timer_ = time; }
		float getCoyoteTimer() const { return coyote_timer_; }

		/**
		 * @brief 切换玩家状态
		 * @param new_state 新状态
		 */
		void setState(std::unique_ptr<state::PlayerState> new_state);

		/**
		 * @brief 处理统一的移动输入逻辑 (施加力、翻转精灵、限制反向速度)。
		 * @param context 引擎上下文。
		 * @param speed_scale 力的缩放系数（如空中移动可能只需要 0.5f）。
		 * @return bool 如果有移动输入（Left 或 Right 按下）则返回 true。
		 */
		bool processMovementInput(engine::core::Context& context, float speed_scale = 1.0f);

		// ========== 动作接口（供命令模式调用） ==========

		/**
		 * @brief 向左移动
		 * @param context 引擎上下文
		 */
		void moveLeft(engine::core::Context& context);

		/**
		 * @brief 向右移动
		 * @param context 引擎上下文
		 */
		void moveRight(engine::core::Context& context);

		/**
		 * @brief 跳跃
		 * @param context 引擎上下文
		 */
		void jump(engine::core::Context& context);

		/**
		 * @brief 攻击
		 * @param context 引擎上下文
		 */
		void attack(engine::core::Context& context);

		/**
		 * @brief 向上攀爬
		 * @param context 引擎上下文
		 */
		void climbUp(engine::core::Context& context);

		/**
		 * @brief 向下攀爬
		 * @param context 引擎上下文
		 */
		void climbDown(engine::core::Context& context);

		/**
		 * @brief 停止移动
		 * @param context 引擎上下文
		 */
		void stopMove(engine::core::Context& context);

	private:
		void init() override;                                                  ///< 初始化组件
		void update(float delta_time, engine::core::Context& context) override; ///< 更新玩家逻辑
	};
}
