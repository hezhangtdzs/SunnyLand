#pragma once
#include "../../engine/object/game_object.h"
#include<memory>
#include "state/player_state.h"
namespace engine::component {
	class TransformComponent;
	class SpriteComponent;
	class PhysicsComponent;
	class AnimationComponent;
}
namespace game::component
{
	namespace state
	{
		class PlayerState;
	}
	class PlayerComponent : public engine::component::Component {
		friend class engine::object::GameObject;
	private:
		engine::component::TransformComponent* transform_component_{ nullptr };
		engine::component::SpriteComponent* sprite_component_{ nullptr };
		engine::component::PhysicsComponent* physics_component_{ nullptr };
		engine::component::AnimationComponent* animation_component_ = nullptr;

		std::unique_ptr<state::PlayerState> current_state_{ nullptr };
		bool is_dead_{ false };

		float move_force_{ 200.0f }; 
		float jump_force_{ 350.0f };
		float max_move_speed_{ 150.0f };
		float  friction_{ 0.8f };
	public:
		PlayerComponent()=default;
		~PlayerComponent()=default;
		PlayerComponent(const PlayerComponent&) = delete;
		PlayerComponent& operator=(const PlayerComponent&) = delete;
		PlayerComponent(PlayerComponent&&) = delete;
		PlayerComponent& operator=(PlayerComponent&&) = delete;

		engine::component::TransformComponent* getTransformComponent() const { return transform_component_; }
		engine::component::SpriteComponent* getSpriteComponent() const { return sprite_component_; }
		engine::component::PhysicsComponent* getPhysicsComponent() const { return physics_component_; }
		engine::component::AnimationComponent* getAnimationComponent() const;

		bool isDead() const { return is_dead_; }
		void setDead(bool is_dead) { is_dead_ = is_dead; }
		float getMoveForce() const { return move_force_; }
		float getJumpForce() const { return jump_force_; }
		float getMaxMoveSpeed() const { return max_move_speed_; }
		float getFriction() const { return friction_; }

		void setState(std::unique_ptr<state::PlayerState> new_state);

		/**
		 * @brief 处理统一的移动输入逻辑 (施加力、翻转精灵、限制反向速度)。
		 * @param context 引擎上下文。
		 * @param speed_scale 力的缩放系数（如空中移动可能只需要 0.5f）。
		 * @return bool 如果有移动输入（Left 或 Right 按下）则返回 true。
		 */
		bool processMovementInput(engine::core::Context& context, float speed_scale = 1.0f);
	private:
		void init() override;
		void handleInput(engine::core::Context& context) override;
		void update(float delta_time, engine::core::Context& context) override;

	};
}