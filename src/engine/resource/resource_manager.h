#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>

// Forward declarations of SDL and SDL-related types in global namespace
struct SDL_Renderer;
struct SDL_Texture;
struct MIX_Audio;
struct TTF_Font;

namespace engine::resource {
	class TextureManager;
	class FontManager;
	class AudioManager;

	class ResourceManager {
	private:
		std::unique_ptr<TextureManager> texture_manager_;
		std::unique_ptr<FontManager> font_manager_;
		std::unique_ptr<AudioManager> audio_manager_;
	public:

		explicit ResourceManager(SDL_Renderer* renderer);
		~ResourceManager();

		void clear();

        // --- 统一资源访问接口 ---
    // -- Texture --
        SDL_Texture* loadTexture(const std::string& file_path);     ///< @brief 载入纹理资源
        SDL_Texture* getTexture(const std::string& file_path);      ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载
        void unloadTexture(const std::string& file_path);          ///< @brief 卸载指定的纹理资源
        glm::vec2 getTextureSize(const std::string& file_path);    ///< @brief 获取指定纹理的尺寸
        void clearTextures();                                      ///< @brief 清空所有纹理资源

        // -- Sound Effects (Chunks) --
        MIX_Audio* loadSound(const std::string& file_path);         ///< @brief 载入音效资源
        MIX_Audio* getSound(const std::string& file_path);          ///< @brief 尝试获取已加载音效的指针，如果未加载则尝试加载
        void unloadSound(const std::string& file_path);             ///< @brief 卸载指定的音效资源
        void clearSounds();                                         ///< @brief 清空所有音效资源

        // -- Music --
        MIX_Audio* loadMusic(const std::string& file_path);         ///< @brief 载入音乐资源
        MIX_Audio* getMusic(const std::string& file_path);          ///< @brief 尝试获取已加载音乐的指针，如果未加载则尝试加载
        void unloadMusic(const std::string& file_path);             ///< @brief 卸载指定的音乐资源
        void clearMusic();                                          ///< @brief 清空所有音乐资源
        void cleanAudio();

        // -- Fonts --
        TTF_Font* loadFont(const std::string& file_path, int point_size);     ///< @brief 载入字体资源
        TTF_Font* getFont(const std::string& file_path, int point_size);      ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
        void unloadFont(const std::string& file_path, int point_size);        ///< @brief 卸载指定的字体资源
        void clearFonts();                                                  ///< @brief 清空所有字体资源
    };
}