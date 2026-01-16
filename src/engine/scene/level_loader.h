#pragma once
#include<nlohmann/json_fwd.hpp>
#include<string>
namespace engine::scene {
	class Scene;
	/**
	 * @class LevelLoader
	 * @brief 关卡加载器，负责从外部文件加载和解析游戏关卡。
	 * 
	 * 该类目前主要支持解析 Tiled 编辑器导出的 JSON 格式 (.tmj) 地图文件。
	 * 它会自动遍历地图中的所有图层，并根据图层类型（如图像层）创建对应的 GameObject 和组件。
	 */
	class LevelLoader final {
		std::string map_path_; ///< 当前正在加载的地图文件的完整路径，用于解析相对资源路径
	public:
		/**
		 * @brief 默认构造函数
		 */
		LevelLoader() = default;

		/**
		 * @brief 从指定路径加载关卡。
		 * 
		 * @param level_path 关卡文件 (.tmj) 的路径
		 * @param scene 目标场景引用，解析出的对象将被添加到该场景中
		 * @return 是否成功加载
		 */
		bool loadLevel(const std::string& level_path, Scene& scene);

	private:
		/** @brief 加载图像图层 (Image Layer) */
		void loadImageLayer(const nlohmann::json& layer_json, Scene& scene);
		/** @brief 加载瓦片图层 (Tile Layer) */
		void loadTileLayer(const nlohmann::json& layer_json, Scene& scene);
		/** @brief 加载对象图层 (Object Group) */
		void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene);

		/**
		 * @brief 将地图文件中的相对路径解析为标准绝对路径。
		 * 
		 * @param image_path 原始相对路径
		 * @return 解析后的路径字符串
		 */
		std::string resolvePath(std::string image_path);

	};
}  // namespace engine::scene