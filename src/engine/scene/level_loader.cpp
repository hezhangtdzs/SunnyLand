#include "level_loader.h"
#include "../component/parallax_component.h"
#include "../component/transform_component.h"
#include "../component/tilelayer_component.h"
#include "../component/sprite_component.h"
#include "../object/game_object.h"
#include "../scene/scene.h"
#include "../core/context.h"
#include "../render/sprite.h"
#include "../utils/math.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>
#include <filesystem>

namespace engine::scene {

    bool LevelLoader::loadLevel(const std::string& level_path, Scene& scene) {
        // 1. 加载 JSON 文件
        std::ifstream file(level_path);
        if (!file.is_open()) {
            spdlog::error("无法打开关卡文件: {}", level_path);
            return false;
        }

        // 2. 解析 JSON 数据
        nlohmann::json json_data;
        try {
            file >> json_data;
        }
        catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析 JSON 数据失败: {}", e.what());
            return false;
        }

        // 3. 获取基本地图信息 (名称、地图尺寸、瓦片尺寸)
        map_path_ = level_path;
        map_size_ = glm::ivec2(json_data.value("width", 0), json_data.value("height", 0));
        tile_size_ = glm::ivec2(json_data.value("tilewidth", 0), json_data.value("tileheight", 0));

        // 4. 加载 tileset 数据
        if (json_data.contains("tilesets") && json_data["tilesets"].is_array()) {
            for (const auto& tileset_json : json_data["tilesets"]) {
                if (!tileset_json.contains("source") || !tileset_json["source"].is_string() ||
                    !tileset_json.contains("firstgid") || !tileset_json["firstgid"].is_number_integer()) {
                    spdlog::error("tilesets 对象中缺少有效 'source' 或 'firstgid' 字段。");
                    continue;
                }
                auto tileset_path = resolvePath(tileset_json["source"], map_path_);  // 支持隐式转换，可以省略.get<T>()方法，
                auto first_gid = tileset_json["firstgid"];
                loadTileset(tileset_path, first_gid);
            }
        }

        // 5. 加载图层数据
        if (!json_data.contains("layers") || !json_data["layers"].is_array()) {       // 地图文件中必须有 layers 数组
            spdlog::error("地图文件 '{}' 中缺少或无效的 'layers' 数组。", level_path);
            return false;
        }
        for (const auto& layer_json : json_data["layers"]) {
            // 获取各图层对象中的类型（type）字段
            std::string layer_type = layer_json.value("type", "none");
            if (!layer_json.value("visible", true)) {
                spdlog::info("图层 '{}' 不可见，跳过加载。", layer_json.value("name", "Unnamed"));
                continue;
            }

            // 根据图层类型决定加载方法
            if (layer_type == "imagelayer") {
                loadImageLayer(layer_json, scene);
            }
            else if (layer_type == "tilelayer") {
                loadTileLayer(layer_json, scene);
            }
            else if (layer_type == "objectgroup") {
                loadObjectLayer(layer_json, scene);
            }
            else {
                spdlog::warn("不支持的图层类型: {}", layer_type);
            }
        }

        spdlog::info("关卡加载完成: {}", level_path);
        return true;
    }

    void LevelLoader::loadImageLayer(const nlohmann::json& layer_json, Scene& scene) {
        // 获取纹理相对路径 （会自动处理'\/'符号）
        const std::string& image_path = layer_json.value("image", "");
        if (image_path.empty()) {
            spdlog::error("图层 '{}' 缺少 'image' 属性。", layer_json.value("name", "Unnamed"));
            return;
        }
        auto texture_id = resolvePath(image_path, map_path_);

        // 获取图层偏移量（json中没有则代表未设置，给默认值即可）
        const glm::vec2 offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));

        // 获取视差因子及重复标志
        const glm::vec2 scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
        const glm::bvec2 repeat = glm::bvec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));

        // 获取图层名称
        const std::string& layer_name = layer_json.value("name", "Unnamed");

        /*  可用类似方法获取其它各种属性，这里我们暂时用不上 */

        // 创建游戏对象
        auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
        // 依次添加Transform，Parallax组件
        game_object->addComponent<engine::component::TransformComponent>(offset);
        game_object->addComponent<engine::component::ParallaxComponent>(texture_id, scroll_factor, repeat);
        // 添加到场景中
        scene.addGameObject(std::move(game_object));
        spdlog::info("加载图层: '{}' 完成", layer_name);
    }

    void LevelLoader::loadTileLayer(const nlohmann::json& layer_json, Scene& scene)
    {
        if (!layer_json.contains("data") || !layer_json["data"].is_array()) {
            spdlog::error("图层 '{}' 缺少 'data' 属性。", layer_json.value("name", "Unnamed"));
            return;
        }
        // 准备 TileInfo Vector (瓦片数量 = 地图宽度 * 地图高度)
        std::vector<engine::component::TileInfo> tiles;
        tiles.reserve(map_size_.x * map_size_.y);

        // 获取图层数据 (瓦片 ID 列表)
        const auto& data = layer_json["data"];

        // 根据gid获取必要信息，并依次填充 TileInfo Vector
        for (const auto& gid : data) {
            tiles.push_back(getTileInfoByGid(gid));
        }

        // 获取图层名称
        const std::string& layer_name = layer_json.value("name", "Unnamed");
        // 创建游戏对象
        auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
        // 添加Tilelayer组件
        game_object->addComponent<engine::component::TileLayerComponent>(tile_size_, map_size_, std::move(tiles));
        // 添加到场景中
        scene.addGameObject(std::move(game_object));
    }

    void LevelLoader::loadObjectLayer(const nlohmann::json & layer_json, Scene & scene)
        {
            if (!layer_json.contains("objects") || !layer_json["objects"].is_array()) {
                spdlog::error("对象图层 '{}' 缺少 'objects' 属性。", layer_json.value("name", "Unnamed"));
                return;
            }

            const auto& objects = layer_json["objects"];
            for (const auto& object : objects) {
                auto gid = object.value("gid", 0);

                if (gid == 0) {
                    // 如果gid为0，代表是自定义形状，如碰撞盒，我们以后再处理
                    // TODO: Handle shapes
                }
                else {
                    // 如果gid存在，则代表这是一个带图像的对象
                    auto tile_info = getTileInfoByGid(gid);
                    if (tile_info.sprite.getTextureId().empty()) {
                        spdlog::error("gid为 {} 的瓦片没有图像纹理。", gid);
                        continue;
                    }

                    // 1. 获取Transform信息
                    auto position = glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f));
                    auto dst_size = glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f));

                    // !! 关键的坐标转换 !!
                    position = glm::vec2(position.x, position.y - dst_size.y);

                    auto rotation = object.value("rotation", 0.0f);

                    // 2. 计算缩放
                    auto src_size_opt = tile_info.sprite.getSourceRect();
                    if (!src_size_opt) {
                        spdlog::error("gid为 {} 的瓦片没有源矩形。", gid);
                        continue;
                    }
                    auto src_size = glm::vec2(src_size_opt->w, src_size_opt->h);
                    auto scale = dst_size / src_size;

                    // 3. 获取对象名称
                    const std::string& object_name = object.value("name", "Unnamed");

                    // 4. 创建GameObject并添加组件
                    auto game_object = std::make_unique<engine::object::GameObject>(object_name);
                    game_object->addComponent<engine::component::TransformComponent>(position, rotation, scale);
                    game_object->addComponent<engine::component::SpriteComponent>(std::move(tile_info.sprite), scene.getContext().getResourceManager());

                    // 5. 添加到场景中
                    scene.addGameObject(std::move(game_object));
                    spdlog::info("加载对象: '{}' 完成", object_name);
                }
            }
        }
    

    engine::component::TileInfo LevelLoader::getTileInfoByGid(int gid)
    {
        // 辅助 lambda: 返回空瓦片，其类型为 EMPTY，这样渲染循环会跳过它
        auto make_empty_tile = []() {
            return engine::component::TileInfo(engine::render::Sprite(), engine::component::TileType::EMPTY);
        };

        if (gid == 0) {
            return make_empty_tile();
        }

        // upper_bound：查找tileset_data_中键大于 gid 的第一个元素，返回迭代器
        auto tileset_it = tileset_data_.upper_bound(gid);
        if (tileset_it == tileset_data_.begin()) {
            // 使用 warn 而不是 error，避免无效 GID（如被删除的瓦片）刷屏
            spdlog::warn("gid为 {} 的瓦片未找到图块集。", gid);
            return make_empty_tile();
        }
        --tileset_it;  // 前移一个位置，这样就得到不大于gid的最近一个元素（我们需要的）

        const auto& tileset = tileset_it->second;
        auto local_id = gid - tileset_it->first;        // 计算瓦片在图块集中的局部ID
        const std::string file_path = tileset.value("file_path", "");       // 获取图块集文件路径
        
        // 图块集分为两种情况，需要分别考虑
        if (tileset.contains("image")) {    // Case 1: 单一图片 (Tileset Image)，例如包含所有瓦片的大图
            std::string image_path = tileset.value("image", "");
            if (image_path.empty()) return make_empty_tile();

            // 获取图片路径
            auto texture_id = resolvePath(image_path, file_path);
            
            // 安全获取 columns，防止除零错误
            int columns = tileset.value("columns", 0);
            if (columns <= 0) {
                 int image_width = tileset.value("imagewidth", 0);
                 int tile_width = tileset.value("tilewidth", tile_size_.x > 0 ? tile_size_.x : 16);
                 if (tile_width > 0 && image_width > 0) columns = image_width / tile_width;
                 else columns = 1; // Fallback
            }

            // 计算瓦片在图片网格中的坐标
            auto coordinate_x = local_id % columns;
            auto coordinate_y = local_id / columns;
            
            // 根据坐标确定源矩形
            SDL_FRect texture_rect = {
                static_cast<float>(coordinate_x * tile_size_.x),
                static_cast<float>(coordinate_y * tile_size_.y),
                static_cast<float>(tile_size_.x),
                static_cast<float>(tile_size_.y)
            };
            engine::render::Sprite sprite{ texture_id, texture_rect };
            auto tile_type = getTileTypeById(tileset, local_id);
			return engine::component::TileInfo(sprite, tile_type);
        }
        else {   // Case 2: 多图片集合 (Collection of Images)，例如每个瓦片是单独的文件
            if (!tileset.contains("tiles")) {   
                return make_empty_tile();
            }
            const auto& tiles_json = tileset["tiles"];
            
            // 标准 Tiled JSON 中 tiles 是数组
            if (tiles_json.is_array()) {
                for (const auto& tile_json : tiles_json) {
                    auto tile_id = tile_json.value("id", -1);
                    if (tile_id == local_id) {   
                        if (!tile_json.contains("image")) return make_empty_tile(); // 仅有数据无图的瓦片

                        std::string image_path = tile_json.value("image", "");
                        auto texture_id = resolvePath(image_path, file_path);
                        
                        auto image_width = tile_json.value("imagewidth", 0);
                        auto image_height = tile_json.value("imageheight", 0);
                        
                        SDL_FRect texture_rect = {
                            static_cast<float>(tile_json.value("x", 0)),
                            static_cast<float>(tile_json.value("y", 0)),
                            static_cast<float>(tile_json.value("width", image_width)),    
                            static_cast<float>(tile_json.value("height", image_height))
                        };
                        engine::render::Sprite sprite{ texture_id, texture_rect };
                        auto tile_type = getTileType(tile_json);
                        return engine::component::TileInfo(sprite, tile_type);
                    }
                }
            }
            // 某些导出设置下 tiles 可能是对象 (ID -> TileObject)
            else if (tiles_json.is_object()) {
                 std::string id_str = std::to_string(local_id);
                 if (tiles_json.contains(id_str)) {
                     const auto& tile_json = tiles_json[id_str];
                     if (tile_json.contains("image")) {
                         std::string image_path = tile_json.value("image", "");
                         auto texture_id = resolvePath(image_path, file_path);
                         
                         SDL_FRect texture_rect = {
                             static_cast<float>(tile_json.value("x", 0)),
                             static_cast<float>(tile_json.value("y", 0)),
                             static_cast<float>(tile_json.value("width", tile_json.value("imagewidth", 0))),
                             static_cast<float>(tile_json.value("height", tile_json.value("imageheight", 0)))
                         };
                         engine::render::Sprite sprite{ texture_id, texture_rect };
                         return engine::component::TileInfo(sprite, engine::component::TileType::NORMAL);
                     }
                 }
            }
        }
        
        // 如果能走到这里，说明查找失败，返回空瓦片而不是报错
        return make_empty_tile();
    }

    void LevelLoader::loadTileset(const std::string& tileset_path, int first_gid)
    {
        std::ifstream tileset_file(tileset_path);
        if (!tileset_file.is_open()) {
            spdlog::error("无法打开 Tileset 文件: {}", tileset_path);
            return;
        }

        nlohmann::json ts_json;
        try {
            tileset_file >> ts_json;
        }
        catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析 Tileset JSON 文件 '{}' 失败: {} (at byte {})", tileset_path, e.what(), e.byte);
            return;
        }
        ts_json["file_path"] = tileset_path;    // 将文件路径存储到json中，后续解析图片路径时需要
        tileset_data_[first_gid] = std::move(ts_json);
        spdlog::info("Tileset 文件 '{}' 加载完成，firstgid: {}", tileset_path, first_gid);
    }

    std::string LevelLoader::resolvePath(const std::string& relative_path, const std::string& file_path)
    {
        try {
            // 获取地图文件的父目录（相对于可执行文件） "assets/maps/level1.tmj" -> "assets/maps"
            auto map_dir = std::filesystem::path(file_path).parent_path();
            // 合并路径（相对于可执行文件）并返回。 /* std::filesystem::canonical：解析路径中的当前目录（.）和上级目录（..）导航符，
                                              /*  得到一个干净的路径 */
            auto final_path = std::filesystem::canonical(map_dir / relative_path);
            return final_path.string();
        }
        catch (const std::exception& e) {
            spdlog::error("解析路径失败: {}", e.what());
            return relative_path;
        }
    }

    engine::component::TileType LevelLoader::getTileType(const nlohmann::json& tile_json)
    {
        if (tile_json.contains("properties")) {
            for (const auto& property : tile_json["properties"]) {
                if (property.value("name", "") == "solid") {
                    return property.value("value", false) ? engine::component::TileType::SOLID : engine::component::TileType::NORMAL;
                }
            }
        }
        return engine::component::TileType::NORMAL;
    }

    engine::component::TileType LevelLoader::getTileTypeById(const nlohmann::json& tileset, int local_id) const
    {
        if (tileset.contains("tiles")) {
            for (const auto& tile : tileset["tiles"]) {
                if (tile.value("id", -1) == local_id) {
                    return const_cast<LevelLoader*>(this)->getTileType(tile);
                }
            }
        }
        return engine::component::TileType::NORMAL;
    }

} // namespace engine::scene