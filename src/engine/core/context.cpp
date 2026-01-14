#include "context.h"
#include "../input/input_manager.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../resource/resource_manager.h"
#include<spdlog/spdlog.h>
engine::core::Context::Context(engine::render::Renderer& renderer, 
							   engine::render::Camera& camera, 
							   engine::resource::ResourceManager& resource_manager, 
							   engine::input::InputManager& input_manager)
							 : renderer_(renderer),
							   camera_(camera),
							   resource_manager_(resource_manager),
							   input_manager_(input_manager)
{
	spdlog::info("Context created.");
}
