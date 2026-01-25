#include "level_loader.h"
#include "../component/parallax_component.h"
#include "../component/transform_component.h"
#include "../component/tilelayer_component.h"
#include "../component/sprite_component.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/animation_component.h"
#include "../component/health_component.h"
#include "../physics/collider.h"
#include "../object/game_object.h"
#include "../scene/scene.h"
#include "../core/context.h"
#include "../resource/resource_manager.h"
#include "../render/sprite.h"
#include "../render/animation.h"
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

        const glm::ivec2 layer_map_size(layer_json.value("width", 0), layer_json.value("height", 0));
        if (layer_map_size.x <= 0 || layer_map_size.y <= 0) {
            spdlog::error("图层 '{}' 缺少或无效的 width/height。", layer_json.value("name", "Unnamed"));
            return;
        }

        const glm::vec2 layer_offset(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));

        // 准备 TileInfo Vector (瓦片数量 = 地图宽度 * 地图高度)
        std::vector<engine::component::TileInfo> tiles;
        tiles.reserve(static_cast<size_t>(layer_map_size.x) * static_cast<size_t>(layer_map_size.y));

        // 获取图层数据 (瓦片 ID 列表)
        const auto& data = layer_json["data"];

        // 根据gid获取必要信息，并依次填充 TileInfo Vector
        for (const auto& gid : data) {
            tiles.push_back(getTileInfoByGid(gid));
        }

        if (tiles.size() != static_cast<size_t>(layer_map_size.x) * static_cast<size_t>(layer_map_size.y)) {
            spdlog::warn("图层 '{}' 的瓦片数据数量({})与 width*height({})不一致。", layer_json.value("name", "Unnamed"),
                tiles.size(), static_cast<size_t>(layer_map_size.x) * static_cast<size_t>(layer_map_size.y));
        }

        // 获取图层名称
        const std::string& layer_name = layer_json.value("name", "Unnamed");
        // 创建游戏对象
        auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
        // 始终添加 TransformComponent，即使 offset 为 0，保证渲染和逻辑一致性
        game_object->addComponent<engine::component::TransformComponent>(layer_offset);
        
        // 添加Tilelayer组件
        game_object->addComponent<engine::component::TileLayerComponent>(tile_size_, layer_map_size, std::move(tiles));
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
                    auto tile_data = getTileDataByGid(gid);
                    auto& tile_info = tile_data.info;
                    auto* tile_json = tile_data.json_ptr;

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
                    
                    // 可以在这里处理 tile_json 中的自定义属性，例如碰撞盒等
                    if (tile_info.type == engine::component::TileType::SOLID && tile_json) {
                        auto collider = std::make_unique<engine::physics::AABBCollider>(src_size);
                        game_object->addComponent<engine::component::ColliderComponent>(std::move(collider));
						game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
                        game_object->setTag("solid");
					}
					else if (tile_info.type == engine::component::TileType::HAZARD && tile_json) {
						auto rect = getCollisionRect(tile_json);
						auto collider_size = rect.has_value() ? rect->size : src_size;
						auto collider = std::make_unique<engine::physics::AABBCollider>(collider_size);
						auto collider_component = game_object->addComponent<engine::component::ColliderComponent>(std::move(collider));
						if (rect.has_value()) collider_component->setOffset(rect->position);
						game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
						game_object->setTag("hazard");
					}
					else if (auto rect = getCollisionRect(tile_json); rect.has_value()) {

                        auto collider = std::make_unique<engine::physics::AABBCollider>(rect->size);
                        auto collider_component = game_object->addComponent<engine::component::ColliderComponent>(std::move(collider));
                        collider_component->setOffset(rect->position);
						game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
                    }
                    if (tile_json) {
                        if (auto tag = getTileProperty<std::string>(*tile_json, "tag"); tag) {
                            game_object->setTag(tag.value());
                        }
                        if (auto gravity = getTileProperty<bool>(*tile_json, "gravity"); gravity) {
                            auto* pc = game_object->getComponent<engine::component::PhysicsComponent>();
                            if (pc) {
                                pc->setUseGravity(gravity.value());
                            }
                            else { // 如果对象还没有物理组件，则为其添加一个
                                game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), gravity.value());
                            }
                        }
                        auto anim_string = getTileProperty<std::string>(*tile_json, "animation");
                        if (anim_string) {
                            // 解析string为JSON对象
                            nlohmann::json anim_json;
                            try {
                                anim_json = nlohmann::json::parse(anim_string.value());
                            }
                            catch (const nlohmann::json::parse_error& e) {
                                spdlog::error("解析动画 JSON 字符串失败: {}", e.what());
                                continue;  // 跳过此对象
                            }
                            // 添加AnimationComponent
                            auto* ac = game_object->addComponent<engine::component::AnimationComponent>();
                            // 添加动画到 AnimationComponent
                            addAnimationFromTileJson(ac, anim_json, src_size);
                        }
						if (auto health = getTileProperty<int>(*tile_json, "health"); health) {
							game_object->addComponent<engine::component::HealthComponent>(health.value());
                        }


                    }
                    
                    // 5. 添加到场景中
                    scene.addGameObject(std::move(game_object));
                    spdlog::info("加载对象: '{}' 完成", object_name);
                }
            }
        } 
    
    const nlohmann::json* LevelLoader::findTileset(int gid)
    {
        if (gid <= 0) return nullptr;

        // 检查缓存
        if (cache_.data && gid >= cache_.first_gid && (cache_.next_first_gid == -1 || gid < cache_.next_first_gid)) {
            return cache_.data;
        }

        // upper_bound：查找tileset_data_中键大于 gid 的第一个元素
        auto it = tileset_data_.upper_bound(gid);
        if (it == tileset_data_.begin()) {
            return nullptr;
        }

        auto current_it = std::prev(it);
        cache_.first_gid = current_it->first;
        cache_.next_first_gid = (it == tileset_data_.end()) ? -1 : it->first;
        cache_.data = &current_it->second;

        return cache_.data;
    }

    TileData LevelLoader::getTileDataByGid(int gid)
    {
        auto make_empty_data = []() {
            return TileData{ engine::component::TileInfo(engine::render::Sprite(), engine::component::TileType::EMPTY), nullptr };
        };

        const nlohmann::json* tileset_ptr = findTileset(gid);
        if (!tileset_ptr) {
            if (gid != 0) spdlog::warn("gid为 {} 的瓦片未找到图块集。", gid);
            return make_empty_data();
        }

        const auto& tileset = *tileset_ptr;
        auto local_id = gid - cache_.first_gid;
        const std::string file_path = tileset.value("file_path", "");

        if (tileset.contains("image")) {
            // Case 1: 单一图片 (Tileset Image)
            std::string image_path = tileset.value("image", "");
            if (image_path.empty()) return make_empty_data();

            auto texture_id = resolvePath(image_path, file_path);
            
            int columns = tileset.value("columns", 0);
            if (columns <= 0) {
                 int image_width = tileset.value("imagewidth", 0);
                 int tile_width = tileset.value("tilewidth", tile_size_.x > 0 ? tile_size_.x : 16);
                 columns = (tile_width > 0 && image_width > 0) ? image_width / tile_width : 1;
            }

            auto coordinate_x = local_id % columns;
            auto coordinate_y = local_id / columns;
            
            SDL_FRect texture_rect = {
                static_cast<float>(coordinate_x * tile_size_.x),
                static_cast<float>(coordinate_y * tile_size_.y),
                static_cast<float>(tile_size_.x),
                static_cast<float>(tile_size_.y)
            };
            
            engine::render::Sprite sprite{ texture_id, texture_rect };
            
            // 查找瓦片的特定 JSON 配置用于获取类型和碰撞框
            const nlohmann::json* tile_json = nullptr;
            if (tileset.contains("tiles")) {
                const auto& tiles_json = tileset["tiles"];
                if (tiles_json.is_array()) {
                    for (const auto& tj : tiles_json) {
                        if (tj.value("id", -1) == local_id) {
                            tile_json = &tj;
                            break;
                        }
                    }
                }
            }

            auto tile_type = tile_json ? getTileType(*tile_json) : engine::component::TileType::NORMAL;
            return TileData{ engine::component::TileInfo(sprite, tile_type), tile_json };
        }
        else {
            // Case 2: 多图片集合
            if (tileset.contains("tiles")) {
                const auto& tiles_json = tileset["tiles"];
                if (tiles_json.is_array()) {
                    for (const auto& tile_json : tiles_json) {
                        if (tile_json.value("id", -1) == local_id) {
                            if (!tile_json.contains("image")) return make_empty_data();

                            std::string image_path = tile_json.value("image", "");
                            auto texture_id = resolvePath(image_path, file_path);
                            
                            SDL_FRect texture_rect = {
                                static_cast<float>(tile_json.value("x", 0)),
                                static_cast<float>(tile_json.value("y", 0)),
                                static_cast<float>(tile_json.value("width", tile_json.value("imagewidth", 0))),    
                                static_cast<float>(tile_json.value("height", tile_json.value("imageheight", 0)))
                            };
                            return TileData{ engine::component::TileInfo(engine::render::Sprite{ texture_id, texture_rect }, getTileType(tile_json)), &tile_json };
                        }
                    }
                }
            }
        }
        
        return make_empty_data();
    }

    engine::component::TileInfo LevelLoader::getTileInfoByGid(int gid)
    {
        return getTileDataByGid(gid).info;
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
                else if (property.value("name", "") == "unisolid") {
                    return property.value("value", false) ? engine::component::TileType::UNISOLID : engine::component::TileType::NORMAL;
                }
                else if (property.value("name", "") == "slope") {
                    std::string type_str = property.value("value", "");
                    // tileset.tsj 
                    // 
                    // 
                    if (type_str == "0_1" || type_str == "slope_0_1") return engine::component::TileType::SLOPE_0_1;
                    else if (type_str == "1_0" || type_str == "slope_1_0") return engine::component::TileType::SLOPE_1_0;
                    else if (type_str == "0_2" || type_str == "slope_0_2") return engine::component::TileType::SLOPE_0_2;
                    else if (type_str == "2_1" || type_str == "slope_2_1") return engine::component::TileType::SLOPE_2_1;
                    else if (type_str == "1_2" || type_str == "slope_1_2") return engine::component::TileType::SLOPE_1_2;
                    else if (type_str == "2_0" || type_str == "slope_2_0") return engine::component::TileType::SLOPE_2_0;
                }
                else if (property.value("name", "") == "hazard") {
                    return property.value("value", false) ? engine::component::TileType::HAZARD : engine::component::TileType::NORMAL;
                }
                else if (property.value("name", "") == "ladder") {
                    return property.value("value", false) ? engine::component::TileType::LADDER : engine::component::TileType::NORMAL;
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

    std::optional<engine::utils::Rect> LevelLoader::getCollisionRect(const nlohmann::json* tile_json) const
    {
        if (!tile_json || !tile_json->contains("objectgroup")) return std::nullopt;
        auto& objectgroup = (*tile_json)["objectgroup"];
        if (!objectgroup.contains("objects")) return std::nullopt;
        auto& objects = objectgroup["objects"];
        for (const auto& object : objects) {
            auto rect = engine::utils::Rect(glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f)),
                glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f)));
            if (rect.size.x > 0 && rect.size.y > 0) {
                return rect;
            }
        }
        return std::nullopt;
    }

    const nlohmann::json* LevelLoader::getTileJsonByGid(int gid)
    {
        return getTileDataByGid(gid).json_ptr;
    }

    void LevelLoader::addAnimationFromTileJson(engine::component::AnimationComponent* anim_comp, const nlohmann::json& anim_json, glm::vec2& size)
    {
        if (!anim_json.is_object() || !anim_comp) {
            spdlog::error("无效的动画 JSON 或 AnimationComponent 指针。");
            return;
        }

        std::string first_anim;

        for (const auto& anim : anim_json.items()) {
            const std::string& anim_name = anim.key();
            if (first_anim.empty()) first_anim = anim_name;

            const auto& anim_info = anim.value();
            if (!anim_info.is_object()) {
                spdlog::warn("动画 '{}' 的信息无效或为空。", anim_name);
                continue;
            }

            auto duration_ms = anim_info.value("duration", 100);
            auto duration = static_cast<float>(duration_ms) / 1000.0f;
            auto row = anim_info.value("row", 0);
            auto loop = anim_info.value("loop", true); // 支持在 JSON 中定义 loop

            if (!anim_info.contains("frames") || !anim_info["frames"].is_array()) {
                spdlog::warn("动画 '{}' 缺少 'frames' 数组。", anim_name);
                continue;
            }

            auto animation = std::make_unique<engine::render::Animation>(anim_name, loop);

            for (const auto& frame : anim_info["frames"]) {
                if (!frame.is_number_integer()) {
                    spdlog::warn("动画 {} 中 frames 数组格式错误！", anim_name);
                    continue;
                }
                auto column = frame.get<int>();
                SDL_FRect src_rect = {
                    column * size.x,
                    row * size.y,
                    size.x,
                    size.y
                };
                animation->addFrame(src_rect, duration);
            }
            anim_comp->addAnimation(std::move(animation));
        }
    }

} // namespace engine::scene