#include "game_app.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include "Time.h"
#include "../resource/resource_manager.h"

engine::core::GameApp::GameApp() = default;

engine::core::GameApp::~GameApp()
{
	if (!is_running_)
	{
		spdlog::warn("GameApp 被销毁时没有显式关闭。现在关闭。 ...");
		close();
	}
	
}

void engine::core::GameApp::run()
{
	if (!init()) {
		spdlog::error("游戏应用程序初始化失败，无法运行！");
		return;
	}
	time_->setTargetFPS(144);
	time_->setTimeScale(1.0);
	while(is_running_) {
		time_->update();
		float delta_time = time_->getDeltaTime();
		handleEvents();
		update(delta_time);
		render();
		//spdlog::info("delta_time: {}", delta_time);
	}
	close();
}

bool engine::core::GameApp::init()
{
	return initSDL() && initTime() && initResourceManager();

}

void engine::core::GameApp::handleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			is_running_ = false;
		}
	}
}

void engine::core::GameApp::update(float& delta_time)
{
}

void engine::core::GameApp::render()
{
}

void engine::core::GameApp::close()
{
	spdlog::trace("关闭 GameApp ...");
	if (sdl_renderer_ != nullptr) {
		SDL_DestroyRenderer(sdl_renderer_);
		sdl_renderer_ = nullptr;
	}
	if (window_ != nullptr) {
		SDL_DestroyWindow(window_);
		window_ = nullptr;
	}
	resource_manager_.reset();
	SDL_Quit();
	is_running_ = false;
}

bool engine::core::GameApp::initSDL()
{
	spdlog::trace("初始化游戏应用程序...");
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
		return false;
	}

	window_ = SDL_CreateWindow("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE);
	if (window_ == nullptr) {
		spdlog::error("无法创建窗口! SDL错误: {}", SDL_GetError());
		return false;
	}

	sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
	if (sdl_renderer_ == nullptr) {
		spdlog::error("无法创建渲染器! SDL错误: {}", SDL_GetError());
		return false;
	}
	is_running_ = true;
	return true;
}

bool engine::core::GameApp::initTime()
{
	try {
		time_ = std::make_unique<Time>();
	}
	catch (const std::exception& e) {
		spdlog::error("初始化时间管理失败: {}", e.what());
		return false;
	}
	spdlog::trace("时间管理初始化成功。");
	return true;
}

bool engine::core::GameApp::initResourceManager() {
	try {
		resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);
	}
	catch (const std::exception& e) {
		spdlog::error("初始化资源管理器失败: {}", e.what());
		return false;
	}
	spdlog::trace("资源管理器初始化成功。");
	return true;
}
void engine::core::GameApp::testResourceManager()
{
	resource_manager_->getTexture("assets/textures/Actors/eagle-attack.png");
	resource_manager_->getFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
	resource_manager_->getSound("assets/audio/button_click.wav");

	resource_manager_->unloadTexture("assets/textures/Actors/eagle-attack.png");
	resource_manager_->unloadFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
	resource_manager_->unloadSound("assets/audio/button_click.wav");
}