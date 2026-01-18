#pragma once
#include <memory>

// Forward declarations in global namespace
struct SDL_Renderer;
struct SDL_Window;

namespace engine::resource {
    class ResourceManager;
}
namespace engine::render {
    class Renderer;
	class Camera;
}
namespace engine::input {
    class InputManager;
}   
namespace engine::scene
{
	class SceneManager;
}
namespace engine::physics
{
	class PhysicsEngine;
}
namespace engine::core {
    class Time;
	class Config;
	class Context;
    class GameApp final{
    private:
        bool is_running_{false};
        SDL_Renderer* sdl_renderer_{nullptr};
        SDL_Window* window_{nullptr};

        std::unique_ptr<engine::core::Time> time_;
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_;
		std::unique_ptr<engine::render::Renderer> renderer_;
		std::unique_ptr<engine::render::Camera> camera_;
		std::unique_ptr<engine::core::Config> config_;
		std::unique_ptr<engine::input::InputManager> input_manager_;
		std::unique_ptr<engine::core::Context> context_;
		std::unique_ptr<engine::scene::SceneManager> scene_manager_;
		std::unique_ptr<engine::physics::PhysicsEngine> physics_engine_;

    public:
        GameApp();
        ~GameApp();
		//禁止拷贝构造和移动
		GameApp(const GameApp&) = delete;
		GameApp& operator=(const GameApp&) = delete;
		GameApp(GameApp&&) = delete;
		GameApp& operator=(GameApp&&) = delete;

        void run();
	private:
        [[nodiscard]] bool init();
		void handleEvents();
		void update(float& delta_time);
		void render();
		void close();

		[[nodiscard]] bool initConfig();
        [[nodiscard]] bool initSDL();
		[[nodiscard]] bool initTime();
		[[nodiscard]] bool initResourceManager();
        [[nodiscard]] bool initRenderer();
        [[nodiscard]] bool initCamera();
		[[nodiscard]] bool initInputManager();
		[[nodiscard]] bool initContext();
		[[nodiscard]] bool initSceneManager();
		[[nodiscard]] bool initPhysicsEngine();
    };
}