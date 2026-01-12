#pragma once
#include <memory>       // 用于 std::unique_ptr
#include <stdexcept>    // 用于 std::runtime_error
#include <string>       // 用于 std::string
#include <unordered_map> // 用于 std::unordered_map
#include <utility>      // 用于 std::pair
#include <functional>   // 用于 std::hash

#include <SDL3_ttf/SDL_ttf.h> // SDL_ttf 主头文件
namespace engine::resource {
	class FontManager final {
	private:
		struct FontKey {
			std::string file_path;
			int point_size;
			bool operator==(const FontKey& other) const {
				return file_path == other.file_path && point_size == other.point_size;
			}
		};
		struct FontKeyHasher {
			std::size_t operator()(const FontKey& key) const {
				return std::hash<std::string>()(key.file_path) ^ std::hash<int>()(key.point_size);
			}
		};
		struct SDLFontDeleter {
			void operator()(TTF_Font* font) const {
				if (font) {
					TTF_CloseFont(font);
				}
			}
		};
		std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHasher> fonts_; ///< 已加载字体的映射表
	public:

		/**
		* @brief 构造函数。初始化 SDL_ttf。
		* @throws std::runtime_error 如果 SDL_ttf 初始化失败。
		*/
		FontManager();

		~FontManager();            ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_ttf。

		// 当前设计中，我们只需要一个FontManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符

		FontManager(const FontManager&) = delete;
		FontManager& operator=(const FontManager&) = delete;
		FontManager(FontManager&&) = delete;
		FontManager& operator=(FontManager&&) = delete;


		TTF_Font* loadFont(const std::string& file_path, int point_size);     ///< @brief 从文件路径加载指定点大小的字体
		TTF_Font* getFont(const std::string& file_path, int point_size);      ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
		void unloadFont(const std::string& file_path, int point_size);        ///< @brief 卸载特定字体（通过路径和大小标识）
		void clearFonts();                                                    ///< @brief 清空所有缓存的字体
	
	};
}