#include"texture_manager.h"
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>  

engine::resource::TextureManager::TextureManager(SDL_Renderer* renderer):renderer_(renderer){
	if (!renderer) {
		throw std::runtime_error("TextureManager initialization failed: renderer is nullptr");
	}
}
SDL_Texture* engine::resource::TextureManager::loadTexture(const std::string& file_path) {
    // 检查是否已加载
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        return it->second.get();
    }

    // 如果没加载则尝试加载纹理
    SDL_Texture* raw_texture = IMG_LoadTexture(renderer_, file_path.c_str());

    if (!raw_texture) {
        spdlog::error("加载纹理失败: '{}': {}", file_path, SDL_GetError());
        return nullptr;
    }

    // 使用带有自定义删除器的 unique_ptr 存储加载的纹理
    textures_.emplace(file_path, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_texture));
    spdlog::debug("成功加载并缓存纹理: {}", file_path);

    return raw_texture;
}
SDL_Texture* engine::resource::TextureManager::getTexture(const std::string& file_path) {
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        return it->second.get();
    }
	spdlog::debug("纹理未缓存，尝试加载: {}", file_path);
    return loadTexture(file_path);
}
void engine::resource::TextureManager::unloadTexture(const std::string& file_path) {
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        textures_.erase(it);
        spdlog::debug("已卸载纹理: {}", file_path);
    } else {
        spdlog::warn("尝试卸载未加载的纹理: {}", file_path);
    }
}

glm::vec2 engine::resource::TextureManager::getTextureSize(const std::string& file_path) {
    SDL_Texture* texture = getTexture(file_path);
    if (!texture) {
        spdlog::error("无法获取纹理: {}", file_path);
        return glm::vec2(0);
    }

    // 获取纹理尺寸
    glm::vec2 size;
    if (!SDL_GetTextureSize(texture, &size.x, &size.y)) {
        spdlog::error("无法查询纹理尺寸: {}", file_path);
        return glm::vec2(0);
    }
    return size;
}
void engine::resource::TextureManager::clearTextures() {
	if (textures_.empty()) {
        spdlog::debug("纹理资源已为空，无需清空");
        return;
    }
    textures_.clear();
    spdlog::debug("已清空所有纹理资源");
}