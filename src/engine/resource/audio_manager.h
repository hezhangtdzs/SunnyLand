#pragma once
#include <memory>       // 用于 std::unique_ptr
#include <stdexcept>    // 用于 std::runtime_error
#include <string>       // 用于 std::string
#include <unordered_map> // 用于 std::unordered_map
 #include <SDL3_mixer/SDL_mixer.h>

namespace engine::resource {

	class AudioManager final {
	public:
		struct MixerDeleter {
			void operator()(MIX_Mixer* m) const {
				if (m) MIX_DestroyMixer(m);
			}
		};
		struct MixAudioDeleter {
			void operator()(MIX_Audio* c) const {
				if (c) MIX_DestroyAudio(c);
			}
		};

	private:
		std::unique_ptr<MIX_Mixer, MixerDeleter> mixer_; ///< SDL_mixer 混音器设备指针
		std::unordered_map<std::string, std::unique_ptr<MIX_Audio, MixAudioDeleter>> music_;
		std::unordered_map<std::string, std::unique_ptr<MIX_Audio, MixAudioDeleter>> sounds_;

		float music_volume_ = 1.0f; ///< 音乐音量 (0.0 - 1.0)
		float sound_volume_ = 1.0f; ///< 音效音量 (0.0 - 1.0)

	public:
		/**
		 * @brief 构造函数。初始化 SDL_mixer 并打开音频设备。
		 * @throws std::runtime_error 如果 SDL_mixer 初始化或打开音频设备失败。
		 */
		AudioManager();

		~AudioManager();            ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_mixer。

		// 当前设计中，我们只需要一个AudioManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
		AudioManager(const AudioManager&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;
		AudioManager(AudioManager&&) = delete;
		AudioManager& operator=(AudioManager&&) = delete;

		MIX_Audio* loadSound(const std::string& file_path);     ///< @brief 从文件路径加载音效
		MIX_Audio* getSound(const std::string& file_path);      ///< @brief 尝试获取已加载音效的指针，如果未加载则尝试加载
		void unloadSound(const std::string& file_path);         ///< @brief 卸载指定的音效资源
		void clearSounds();                                      ///< @brief 清空所有音效资源

		MIX_Audio* loadMusic(const std::string& file_path);     ///< @brief 从文件路径加载音乐
		MIX_Audio* getMusic(const std::string& file_path);      ///< @brief 尝试获取已加载音乐的指针，如果未加载则尝试加载
		void unloadMusic(const std::string& file_path);         ///< @brief 卸载指定的音乐资源
		void clearMusic();                                      ///< @brief 清空所有音乐资源

		void clearAudio();                                      ///< @brief 清空所有音频资源

		///**
		// * @brief 设置音乐音量
		// * @param volume 音量大小 (0.0 - 1.0)
		// * @note 这会影响所有标签为 "music" 的正在播放的轨道
		// */
		//void setMusicVolume(float volume);

		///**
		// * @brief 获取当前设置的音乐音量
		// * @return float 音量大小
		// */
		//[[nodiscard]] float getMusicVolume() const { return music_volume_; }

		///**
		// * @brief 设置音效音量
		// * @param volume 音量大小 (0.0 - 1.0)
		// * @note 这会影响所有标签为 "sound" 的正在播放的轨道
		// */
		//void setSoundVolume(float volume);

		///**
		// * @brief 获取当前设置的音效音量
		// * @return float 音量大小
		// */
		//[[nodiscard]] float getSoundVolume() const { return sound_volume_; }
	};
}