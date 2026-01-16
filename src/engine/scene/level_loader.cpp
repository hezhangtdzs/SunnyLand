#include "level_loader.h"
#include <filesystem>
#include <spdlog/spdlog.h>
#include<nlohmann/json.hpp>
#include<glm/glm.hpp>
#include"../object/game_object.h"
#include"../scene/scene.h"
#include"../component/parallax_component.h"
#include"../component/transform_component.h"
#include<fstream>
#include <utility>
bool engine::scene::LevelLoader::loadLevel(const std::string& level_path, Scene& scene)
{
	map_path_ = level_path;
	std::ifstream file(level_path);
	if(!file.is_open()) {
		spdlog::error("无法打开关卡文件: {}", level_path);
		return false;
	}
	nlohmann::json level_json;
	try {
		file >> level_json;
	}
	catch (const nlohmann::json::parse_error& e) {
		spdlog::error("解析关卡文件时出错: {}", e.what());
		return false;
	}
	if (!level_json.contains("layers") || !level_json["layers"].is_array()) {
		spdlog::error("关卡文件格式错误: 缺少 'layers' 数组");
		return false;
	}
	for (const auto& layer_json : level_json["layers"]) {
		if(!layer_json.contains("type") || !layer_json["type"].is_string()) {
			spdlog::warn("跳过无效图层: 缺少 'type' 字段");
			continue;
		}
		if (!layer_json.value("visible", true)) {
			spdlog::info("图层 '{}' 不可见，跳过加载。", layer_json.value("name", "Unnamed"));
			continue;
		}
		std::string layer_type = layer_json.value("type","none");
		if(layer_type == "imagelayer") {
			loadImageLayer(layer_json, scene);
		}
		else if(layer_type == "tilelayer") {
			loadTileLayer(layer_json, scene);
		}
		else if(layer_type == "objectgroup") {
			loadObjectLayer(layer_json, scene);
		}
		else {
			spdlog::warn("未知图层类型: {}", layer_type);
		}
		spdlog::info("成功加载图层: {}", layer_json.value("name", "Unnamed"));
	}

	return true;
}
void engine::scene::LevelLoader::loadImageLayer(const nlohmann::json& layer_json, Scene& scene)
{
	std::string image_path = layer_json.value("image", "");
	if (image_path.empty()) {
		spdlog::warn("图层 '{}' 缺少图片路径，跳过加载。", layer_json.value("name", "Unnamed"));
		return;
	}
	auto texture_id = resolvePath(image_path);

	// 修复 1: 同时读取 x/y 和 offsetx/offsety 并相加
	// Tiled 中的图像层位置由 x, y 和 偏移量 offsetx, offsety 共同决定
	float x = layer_json.value("x", 0.0f);
	float y = layer_json.value("y", 0.0f);
	float offset_x = layer_json.value("offsetx", 0.0f);
	float offset_y = layer_json.value("offsety", 0.0f);
	
	glm::vec2 position = { x + offset_x, y + offset_y };

	std::string layer_name = layer_json.value("name", "Unnamed");
	glm::vec2 parallax_factor = {
		layer_json.value("parallaxx", 1.0f),
		layer_json.value("parallaxy", 1.0f)
	};

	// 修复 2: 将默认值改为 false
	// 如果 JSON 中不存在 repeat 字段，Tiled 默认为不重复
	glm::bvec2 repeat = {
		layer_json.value("repeatx", false),
		layer_json.value("repeaty", false)
	};

	auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
	// 使用计算后的最终位置
	game_object->addComponent<engine::component::TransformComponent>(position);
	game_object->addComponent<engine::component::ParallaxComponent>(texture_id, parallax_factor, repeat);
	scene.addGameObject(std::move(game_object));
	spdlog::info("加载图层: '{}' 完成", layer_name);
}
void engine::scene::LevelLoader::loadTileLayer(const nlohmann::json& layer_json, Scene& scene)
{
}
void engine::scene::LevelLoader::loadObjectLayer(const nlohmann::json& layer_json, Scene& scene)
{
}
std::string engine::scene::LevelLoader::resolvePath(std::string image_path)
{
	try {
		auto map_dir = std::filesystem::path(map_path_).parent_path();
		auto final_path = std::filesystem::canonical(map_dir / image_path);
		return final_path.string();
	}
	catch (const std::filesystem::filesystem_error& e) {
		spdlog::error("解析路径时出错: {}", e.what());
		return image_path; // 返回原始路径作为回退
	}
}
