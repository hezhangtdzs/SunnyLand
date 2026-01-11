#pragma once
#include<memory>
class SDL_Renderer;
class SDL_Window;

namespace engine::core {
	class Time;
    class GameApp final{
	private:
		bool is_running_{false};
		SDL_Renderer* sdl_renderer_{nullptr};
		SDL_Window* window_{nullptr};
		std::unique_ptr<engine::core::Time> time_;
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
    };
}