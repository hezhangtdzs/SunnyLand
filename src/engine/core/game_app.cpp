#include "game_app.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include "Time.h"
#include "../resource/resource_manager.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "config.h"
#include "../input/input_manager.h"
#include "../object/game_object.h"
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
	time_->setTargetFPS(config_->target_fps_);
	time_->setTimeScale(1.0);
	while(is_running_) {
		time_->update();
		float delta_time = time_->getDeltaTime();
		input_manager_->Update();
		handleEvents();
		update(delta_time);
		render();
		//spdlog::info("delta_time: {}", delta_time);
	}
	close();
}

bool engine::core::GameApp::init()
{
	if (initConfig() &&
		initSDL() &&
		initInputManager() &&
		initTime() && 
		initResourceManager()&&
		initRenderer()&&
		initCamera()) {
		spdlog::info("游戏应用程序初始化成功。");
		testResourceManager();
		testGameObject();
		return true;
	}

	spdlog::error("游戏应用程序初始化失败。");
	return false;
	
}

void engine::core::GameApp::handleEvents()
{
	if (input_manager_->shouldQuit()) {
		spdlog::trace("GameApp 收到来自 InputManager 的退出请求。");
		is_running_ = false;
		return;
	}
	testInputManager();
}

void engine::core::GameApp::update(float& delta_time)
{
	// 游戏逻辑更新
	testCamera(delta_time);
}

void engine::core::GameApp::render()
{
	// 1. 清除屏幕
	renderer_->clearScreen();

	// 2. 具体渲染代码
	testRenderer();

	// 3. 更新屏幕显示
	renderer_->present();
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

bool engine::core::GameApp::initConfig()
{
	try{
		config_ = std::make_unique<engine::core::Config>("assets/config.json");
	}
	catch (const std::exception& e)
	{
		spdlog::error("初始化配置失败: {}", e.what());
		return false;
	}
	spdlog::trace("配置初始化成功。");
	return true;
}

bool engine::core::GameApp::initSDL()
{
	spdlog::trace("初始化游戏应用程序...");
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
		return false;
	}

	window_ = SDL_CreateWindow(config_->window_title_.c_str(),config_->window_width_, config_->window_height_, SDL_WINDOW_RESIZABLE);
	if (window_ == nullptr) {
		spdlog::error("无法创建窗口! SDL错误: {}", SDL_GetError());
		return false;
	}

	sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
	if (sdl_renderer_ == nullptr) {
		spdlog::error("无法创建渲染器! SDL错误: {}", SDL_GetError());
		return false;
	}
	int vsync_mode = config_->vsync_enabled_ ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
	SDL_SetRenderVSync(sdl_renderer_, vsync_mode);
	
	SDL_SetRenderLogicalPresentation(sdl_renderer_, config_->window_width_/2, config_->window_height_/2, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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
bool engine::core::GameApp::initRenderer()
{
	try {
		renderer_ = std::make_unique<engine::render::Renderer>(sdl_renderer_, resource_manager_.get());
	}
	catch (const std::exception& e) {
		spdlog::error("初始化渲染器失败: {}", e.what());
		return false;
	}
	spdlog::trace("渲染器初始化成功。");
	return true;
}
bool engine::core::GameApp::initCamera()
{
	try {
		camera_ = std::make_unique<engine::render::Camera>( glm::vec2(config_->window_width_/2, config_->window_height_/2));

	}
	catch (const std::exception& e) {
		spdlog::error("初始化相机失败: {}", e.what());
		return false;
	}
	spdlog::trace("相机初始化成功。");
	return true;
}
bool engine::core::GameApp::initInputManager()
{
	try {
		input_manager_ = std::make_unique<engine::input::InputManager>(sdl_renderer_, config_.get());
	}
	catch (const std::exception& e) {
		spdlog::error("初始化输入管理器失败: {}", e.what());
		return false;
	}
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

void engine::core::GameApp::testRenderer()
{
	engine::render::Sprite sprite_world("assets/textures/Actors/frog.png");
	engine::render::Sprite sprite_ui("assets/textures/UI/buttons/Start1.png");
	engine::render::Sprite sprite_parallax("assets/textures/Layers/back.png");

	static float rotation = 0.0f;
	rotation += 0.1f;

	// 注意渲染顺序
	renderer_->drawParallax(*camera_, sprite_parallax, glm::vec2(100, 100), glm::vec2(0.5f, 0.5f), glm::bvec2(true, false));
	renderer_->drawSprite(*camera_, sprite_world, glm::vec2(200, 200), glm::vec2(1.0f, 1.0f), rotation);
	renderer_->drawUISprite(sprite_ui, glm::vec2(100, 100));

}

void engine::core::GameApp::testCamera(float delta_time)
{
	const float speed = 200.0f; // pixels per second
	auto key_state = SDL_GetKeyboardState(nullptr);
	if (key_state[SDL_SCANCODE_UP]) camera_->move(glm::vec2(0, -1) * speed * delta_time);
	if (key_state[SDL_SCANCODE_DOWN]) camera_->move(glm::vec2(0, 1) * speed * delta_time);
	if (key_state[SDL_SCANCODE_LEFT]) camera_->move(glm::vec2(-1, 0) * speed * delta_time);
	if (key_state[SDL_SCANCODE_RIGHT]) camera_->move(glm::vec2(1, 0) * speed * delta_time);
}

void engine::core::GameApp::testInputManager()
{
	std::vector<std::string> actions = {
		"move_up",
		"move_down",
		"move_left",
		"move_right",
		"jump",
		"attack",
		"pause",
		"MouseLeftClick",
		"MouseRightClick"
	};

	for (const auto& action : actions) {
		if (input_manager_->isActionPressed(action)) {
			spdlog::info(" {} 按下 ", action);
		}
		if (input_manager_->isActionReleased(action)) {
			spdlog::info(" {} 抬起 ", action);
		}
		if (input_manager_->isActionDown(action)) {
			spdlog::info(" {} 按下中 ", action);
		}
	}
}
void engine::core::GameApp::testGameObject()
{
	engine::object::GameObject game_object("test_game_object");
	game_object.addComponent<engine::component::Component>();
}