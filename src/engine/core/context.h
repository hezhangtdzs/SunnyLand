#pragma once

namespace engine::input{
	class InputManager;
}
namespace engine::resource
{
	class ResourceManager;
}
namespace engine::render
{
	class Renderer;
	class Camera;
}
namespace engine::physics
{
	class PhysicsEngine;
}
namespace engine::core
{
	class Context final
	{
	private:
			engine::resource::ResourceManager& resource_manager_;
			engine::render::Renderer& renderer_;
			engine::render::Camera& camera_;
			engine::input::InputManager& input_manager_;
			engine::physics::PhysicsEngine& physics_engine_;
	public:
		Context(engine::render::Renderer& renderer,
				engine::render::Camera& camera,
				engine::resource::ResourceManager& resource_manager,
				engine::input::InputManager& input_manager,
				engine::physics::PhysicsEngine& physics_engine);
			

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		//getter
		engine::render::Renderer& getRenderer()
		{
			return renderer_;
		}
		engine::render::Camera& getCamera()
		{
			return camera_;
		}
		engine::resource::ResourceManager& getResourceManager()
		{
			return resource_manager_;
		}
		engine::input::InputManager& getInputManager()
		{
			return input_manager_;
		}
		engine::physics::PhysicsEngine& getPhysicsEngine()
		{
			return physics_engine_;
		}
	};

}