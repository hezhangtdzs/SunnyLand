#pragma once
#include <memory>       // 用于 std::unique_ptr
#include <stdexcept>    // 用于 std::runtime_error
#include <string>       // 用于 std::string
#include <unordered_map> // 用于 std::unordered_map
#include <SDL3/SDL_render.h> // 用于 SDL_Texture 和 SDL_Renderer
#include <glm/glm.hpp>
namespace engine::resource {

	class TextureManager final {
	private:
		struct SDLTextureDeleter {
			void operator()(SDL_Texture* texture) const {
				if (texture) {
					SDL_DestroyTexture(texture);
				}
			}
		};
		SDL_Renderer* renderer_; ///< 指向 SDL_Renderer 上下文的指针
		std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_; ///< 已加载纹理的映射表
	public:
		/**
		 * @brief 构造函数，执行初始化。
		 * @param renderer 指向有效的 SDL_Renderer 上下文的指针。不能为空。
		 * @throws std::runtime_error 如果 renderer 为 nullptr 或初始化失败。
		 */
		explicit TextureManager(SDL_Renderer* renderer);

		// 当前设计中，我们只需要一个TextureManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;
		TextureManager(TextureManager&&) = delete;
		TextureManager& operator=(TextureManager&&) = delete;

		SDL_Texture* loadTexture(const std::string& file_path);     ///< @brief 载入纹理资源
		SDL_Texture* getTexture(const std::string& file_path);      ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载
		void unloadTexture(const std::string& file_path);          ///< @brief 卸载指定的纹理资源
		glm::vec2 getTextureSize(const std::string& file_path);    ///< @brief 获取指定纹理的尺寸
		void clearTextures();
	};
}