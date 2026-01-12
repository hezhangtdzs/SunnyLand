#include "audio_manager.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <stdexcept>
#include <string>
#include <algorithm> // for std::clamp
#include <spdlog/spdlog.h>

namespace engine::resource {

    AudioManager::AudioManager() {
        // 1. 初始化 (SDL3 中 MIX_Init 不需要参数)
        if (!MIX_Init()) {
            throw std::runtime_error("SDL_Mixer 初始化失败: " + std::string(SDL_GetError()));
        }

        // 2. 创建混音器设备
        // 使用默认播放设备
        SDL_AudioDeviceID device_id = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
        mixer_.reset(MIX_CreateMixerDevice(device_id, NULL));

        if (!mixer_) {
            throw std::runtime_error("SDL_Mixer 打开音频失败: " + std::string(SDL_GetError()));
        }

        // 3. 设置主音量 (SDL3 范围是 0.0 - 1.0)
        MIX_SetMasterGain(mixer_.get(), 0.25f);
        
        spdlog::trace("AudioManager 构造成功。");
    }

    AudioManager::~AudioManager() {
        if (mixer_) {
            // 停止所有主要标签的轨道
			// 注意：这需要在播放时正确设置标签 (MIX_TagTrack)
            MIX_StopTag(mixer_.get(), "music", 0);
            MIX_StopTag(mixer_.get(), "sound", 0);

            // 清空所有资源，确保 MIX_Audio 资源在 mixer 销毁前释放
            clearAudio();

            // 重要：必须在 MIX_Quit 之前手动 reset mixer
            // 否则 unique_ptr 会在 MIX_Quit 之后析构，导致非法访问
            mixer_.reset();
        }

        // 最后关闭 SDL_mixer
        MIX_Quit();
    }

    // --- 音效管理 (Sound Effects) ---

    MIX_Audio* AudioManager::loadSound(const std::string& file_path) {
        // 1. 检查缓存
        auto it = sounds_.find(file_path);
        if (it != sounds_.end()) {
            return it->second.get();
        }

        spdlog::debug("加载音效: {}", file_path);

        // 2. 加载音效
        // 音效通常较短，使用 predecode = true (预解码) 将 PCM 数据加载到内存中，
        // 以避免播放时的解码开销，保证低延迟。
        MIX_Audio* raw_audio = MIX_LoadAudio(mixer_.get(), file_path.c_str(), true);

        if (!raw_audio) {
            throw std::runtime_error("加载音效失败: " + file_path + " 错误: " + std::string(SDL_GetError()));
        }

        // 3. 存入缓存
        sounds_.emplace(file_path, std::unique_ptr<MIX_Audio, MixAudioDeleter>(raw_audio));
        
        return raw_audio;
    }

    MIX_Audio* AudioManager::getSound(const std::string& file_path) {
        auto it = sounds_.find(file_path);
        if (it != sounds_.end()) {
            return it->second.get();
        }
        spdlog::warn("音效未缓存，尝试直接加载: {}", file_path);
        return loadSound(file_path);
    }

    void AudioManager::unloadSound(const std::string& file_path) {
        auto it = sounds_.find(file_path);
        if (it != sounds_.end()) {
            spdlog::debug("卸载音效: {}", file_path);
            sounds_.erase(it);
        } else {
            spdlog::warn("尝试卸载未加载的音效: {}", file_path);
        }
    }

    void AudioManager::clearSounds() {
        if (!sounds_.empty()) {
            spdlog::debug("正在清除所有 {} 个缓存的音效。", sounds_.size());
            sounds_.clear();
        }
    }

    // --- 音乐管理 (Music) ---

    MIX_Audio* AudioManager::loadMusic(const std::string& file_path) {
        auto it = music_.find(file_path);
        if (it != music_.end()) {
            return it->second.get();
        }

        spdlog::debug("加载音乐: {}", file_path);

        // 2. 加载音乐
        // 关键优化：音乐文件通常较大（如 BGM），将 predecode 设置为 false。
        // 这样会保留源格式（如 mp3/ogg），在播放时流式解码，大幅减少内存占用。
        MIX_Audio* raw_audio = MIX_LoadAudio(mixer_.get(), file_path.c_str(), false);

        if (!raw_audio) {
            throw std::runtime_error("加载音乐失败: " + file_path + " 错误: " + std::string(SDL_GetError()));
        }

        music_.emplace(file_path, std::unique_ptr<MIX_Audio, MixAudioDeleter>(raw_audio));
        return raw_audio;
    }

    MIX_Audio* AudioManager::getMusic(const std::string& file_path) {
        auto it = music_.find(file_path);
        if (it != music_.end()) {
            return it->second.get();
        }
        spdlog::warn("音乐未缓存，尝试直接加载: {}", file_path);
        return loadMusic(file_path);
    }

    void AudioManager::unloadMusic(const std::string& file_path) {
        auto it = music_.find(file_path);
        if (it != music_.end()) {
            spdlog::debug("卸载音乐: {}", file_path);
            music_.erase(it);
        } else {
            spdlog::warn("尝试卸载未加载的音乐: {}", file_path);
        }
    }

    void AudioManager::clearMusic() {
        if (!music_.empty()) {
            spdlog::debug("正在清除所有 {} 个缓存的音乐。", music_.size());
            music_.clear();
        }
    }

    // --- 统一管理 ---

    void AudioManager::clearAudio() {
        clearMusic();
        clearSounds();
    }

    // --- 音量控制 ---

    //void AudioManager::setMusicVolume(float volume) {
    //    music_volume_ = std::clamp(volume, 0.0f, 1.0f);
    //    if (mixer_) {
    //        // 设置标签为 "music" 的所有轨道的增益
    //        // 注意：播放音乐的代码必须使用 MIX_PlayTag 或 MIX_TagTrack 将轨道标记为 "music"
    //        MIX_SetTagGain(mixer_.get(), "music", music_volume_);
    //    }
    //}

    //void AudioManager::setSoundVolume(float volume) {
    //    sound_volume_ = std::clamp(volume, 0.0f, 1.0f);
    //    if (mixer_) {
    //        // 设置标签为 "sound" 的所有轨道的增益
    //        MIX_SetTagGain(mixer_.get(), "sound", sound_volume_);
    //    }
    //}

} // namespace engine::resource