#pragma once

#include <string>
#include <unordered_map>
#include <glm/vec2.hpp>

namespace engine::resource { class ResourceManager; }

namespace engine::audio {
	class AudioPlayer final {
	private:
		engine::resource::ResourceManager& resource_manager_;
		float master_volume_{ 1.0f };
		float sound_volume_{ 1.0f };
		float music_volume_{ 1.0f };
		std::string current_music_;

	public:
		explicit AudioPlayer(engine::resource::ResourceManager& resource_manager);

		AudioPlayer(const AudioPlayer&) = delete;
		AudioPlayer& operator=(const AudioPlayer&) = delete;
		AudioPlayer(AudioPlayer&&) = delete;
		AudioPlayer& operator=(AudioPlayer&&) = delete;

		void setMasterVolume(float volume);
		void setSoundVolume(float volume);
		void setMusicVolume(float volume);

		float getMasterVolume() const { return master_volume_; }
		float getSoundVolume() const { return sound_volume_; }
		float getMusicVolume() const { return music_volume_; }

		int playSound(const std::string& path);
		int playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance);

		bool playMusic(const std::string& path, int loops = -1);
		void stopMusic();
	};
}
