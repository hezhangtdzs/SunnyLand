#include "resource_manager.h"
#include "texture_manager.h"
#include "audio_manager.h"
#include "font_manager.h" 
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

engine::resource::ResourceManager::ResourceManager(SDL_Renderer* renderer)
	: texture_manager_(std::make_unique<TextureManager>(renderer)),
	  font_manager_(std::make_unique<FontManager>()),
	  audio_manager_(std::make_unique<AudioManager>()) {
	spdlog::trace("ResourceManager 构造成功。");
}

engine::resource::ResourceManager::~ResourceManager() = default;


// --- 统一资源访问接口 ---
SDL_Texture* engine::resource::ResourceManager::loadTexture(const std::string& file_path) {
	return texture_manager_->loadTexture(file_path);
}
SDL_Texture* engine::resource::ResourceManager::getTexture(const std::string& file_path) {
	return texture_manager_->getTexture(file_path);
}
void engine::resource::ResourceManager::unloadTexture(const std::string& file_path) {
	texture_manager_->unloadTexture(file_path);
}
glm::vec2 engine::resource::ResourceManager::getTextureSize(const std::string& file_path) {
	return texture_manager_->getTextureSize(file_path);
}
void engine::resource::ResourceManager::clearTextures(){
	texture_manager_->clearTextures();
}


//--- 统一音乐音效访问接口 ---
MIX_Audio* engine::resource::ResourceManager::loadSound(const std::string& file_path) {
	return audio_manager_->loadSound(file_path);
}
void engine::resource::ResourceManager::playSound(const std::string& file_path) {
	audio_manager_->playSound(file_path);
}
void engine::resource::ResourceManager::stopSound() {
	audio_manager_->stopSound();
}
MIX_Audio* engine::resource::ResourceManager::getSound(const std::string& file_path) {
	return audio_manager_->getSound(file_path);
}
void engine::resource::ResourceManager::unloadSound(const std::string& file_path) {
	audio_manager_->unloadSound(file_path);
}
void engine::resource::ResourceManager::clearSounds() {
	audio_manager_->clearSounds();
}
MIX_Audio* engine::resource::ResourceManager::loadMusic(const std::string& file_path) {
	return audio_manager_->loadMusic(file_path);
}
void engine::resource::ResourceManager::playMusic(const std::string& file_path) {
	audio_manager_->playMusic(file_path);
}
void engine::resource::ResourceManager::stopMusic() {
	audio_manager_->stopMusic();
}

void engine::resource::ResourceManager::setMasterGain(float gain) {
	audio_manager_->setMasterGain(gain);
}

void engine::resource::ResourceManager::setSoundGain(float gain) {
	audio_manager_->setSoundGain(gain);
}

void engine::resource::ResourceManager::setMusicGain(float gain) {
	audio_manager_->setMusicGain(gain);
}
MIX_Audio* engine::resource::ResourceManager::getMusic(const std::string& file_path) {
	return audio_manager_->getMusic(file_path);
}
void engine::resource::ResourceManager::unloadMusic(const std::string& file_path) {
	audio_manager_->unloadMusic(file_path);
}
void engine::resource::ResourceManager::clearMusic() {
	audio_manager_->clearMusic();
}

void engine::resource::ResourceManager::cleanAudio()
{
	audio_manager_->clearAudio();
}

//--- 统一字体访问接口 ---
TTF_Font* engine::resource::ResourceManager::loadFont(const std::string& file_path, int point_size) {
	return font_manager_->loadFont(file_path, point_size);
}
TTF_Font* engine::resource::ResourceManager::getFont(const std::string& file_path, int point_size) {
	return font_manager_->getFont(file_path, point_size);
}
void engine::resource::ResourceManager::unloadFont(const std::string& file_path, int point_size) {
	font_manager_->unloadFont(file_path, point_size);
}
void engine::resource::ResourceManager::clearFonts() {
	font_manager_->clearFonts();
}
void engine::resource::ResourceManager::clear() {
	clearTextures();
	clearFonts();
	cleanAudio();
}