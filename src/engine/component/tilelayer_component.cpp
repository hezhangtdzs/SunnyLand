#include "tilelayer_component.h"
#include "../object/game_object.h"
#include "transform_component.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../physics/physics_engine.h"
#include "../render/camera.h"
#include <spdlog/spdlog.h>
#include <glm/ext/vector_int2.hpp> // 修复VCIC001警告

using engine::component::TileType; // 添加此行以修复未定义标识符"TileType"错误

engine::component::TileLayerComponent::TileLayerComponent(const glm::ivec2& tile_size, const glm::ivec2& map_size, const std::vector<TileInfo>& tiles)
:tile_size_(tile_size), map_size_(map_size), tiles_(tiles)
{

	if (tiles_.size() != static_cast<size_t>(map_size_.x * map_size_.y)) {
		spdlog::error("TileLayerComponent: 地图尺寸与提供的瓦片向量大小不匹配。瓦片数据将被清除。");
		tiles_.clear();
		map_size_ = { 0, 0 };
	}
	spdlog::trace("TileLayerComponent 构造完成");
}

engine::component::TileLayerComponent::~TileLayerComponent() = default;

const engine::component::TileInfo& engine::component::TileLayerComponent::getTileAt(const glm::ivec2& tile_coords) const
{
	// 检查坐标合法性
	if (tile_coords.x < 0 || tile_coords.y < 0 ||
		tile_coords.x >= map_size_.x || tile_coords.y >= map_size_.y) {
		static const TileInfo empty_tile(engine::render::Sprite(), TileType::EMPTY);
		return empty_tile;
	}
	size_t index = static_cast<size_t>(tile_coords.y) * static_cast<size_t>(map_size_.x) + static_cast<size_t>(tile_coords.x);
	if (index >= tiles_.size()) {
		static const TileInfo empty_tile(engine::render::Sprite(), TileType::EMPTY);
		return empty_tile;
	}
	return tiles_[index];
}

TileType engine::component::TileLayerComponent::getTileTypeAt(const glm::ivec2& tile_coords) const
{
	// 返回指定瓦片坐标的瓦片类型
	return getTileAt(tile_coords).type;
}

TileType engine::component::TileLayerComponent::getTileTypeAtWorldPos(const glm::vec2& world_pos) const
{
	// 计算世界坐标对应的瓦片坐标
	glm::vec2 layer_world_offset = offset_;
	if (owner_) {
		if (auto* tc = owner_->getComponent<engine::component::TransformComponent>()) {
			layer_world_offset += tc->getPosition();
		}
	}
	glm::ivec2 tile_coords = glm::floor((world_pos - layer_world_offset) / glm::vec2(tile_size_));
	return getTileTypeAt(tile_coords);
}

void engine::component::TileLayerComponent::init()
{
	if (!owner_) {
		spdlog::warn("TileLayerComponent 的 owner_ 未设置。");
	}
	spdlog::trace("TileLayerComponent 初始化完成");
}

void engine::component::TileLayerComponent::render(engine::core::Context& context)
{
	if (is_hidden_) {
		return;
	}
	auto& renderer = context.getRenderer();
	auto& camera = context.getCamera();

	// --- 1. 视锥体剔除 (Culling) ---
	// 目的：仅渲染摄像机视野内的瓦片，极大提高大地图的渲染性能
	
	glm::vec2 cam_pos = camera.getPosition();
	glm::vec2 cam_size = camera.getViewportSize();

	glm::vec2 layer_world_offset = offset_;
	if (owner_) {
		if (auto* tc = owner_->getComponent<engine::component::TransformComponent>()) {
			layer_world_offset += tc->getPosition();
		}
	}

	// 计算视野范围对应的网格坐标 (包含一些冗余量以防边缘闪烁)
	glm::ivec2 start_tile = glm::floor((cam_pos - layer_world_offset) / glm::vec2(tile_size_));
	glm::ivec2 end_tile = glm::ceil((cam_pos + cam_size - layer_world_offset) / glm::vec2(tile_size_));

	// 限制坐标在地图有效范围内 (Intersection)
	start_tile = glm::max(start_tile, glm::ivec2(0));
	end_tile = glm::min(end_tile, map_size_);

	// --- 2. 渲染循环 ---
	for (int y = start_tile.y; y < end_tile.y; ++y) {
		for (int x = start_tile.x; x < end_tile.x; ++x) {
			const TileInfo& tile = getTileAt({ x, y });
			
			if (tile.type != TileType::EMPTY) {
				glm::vec2 tile_world_pos = layer_world_offset + glm::vec2(x * tile_size_.x, y * tile_size_.y);
				
				// --- 3. 底部对齐逻辑 (Bottom Alignment) ---
				// Tiled 规则：如果图片高度 > 瓦片高度（如树木），图片底部应与网格底部对齐
				
				float sprite_h = static_cast<float>(tile_size_.y);
				// 安全访问 optional
				if (tile.sprite.getSourceRect().has_value()) {
					sprite_h = tile.sprite.getSourceRect()->h;
				}

				// 如果高度不一致，向上偏移差异值
				if (std::abs(sprite_h - tile_size_.y) > 0.1f) {
					tile_world_pos.y -= (sprite_h - static_cast<float>(tile_size_.y));
				}

				renderer.drawSprite(camera, tile.sprite, tile_world_pos, glm::vec2(1.0f, 1.0f));
			}
		}
	}
}

void engine::component::TileLayerComponent::update(float deltaTime, engine::core::Context& context)
{
}

void engine::component::TileLayerComponent::clean()
{
	if (physics_engine_) {
		physics_engine_->unregisterCollisionLayer(this);
	}
}
