#include "audio_player.h"

#include "../resource/resource_manager.h"
#include <algorithm>
#include <cmath>

namespace engine::audio {
	static float clamp01(float v) {
		return std::clamp(v, 0.0f, 1.0f);
	}

	AudioPlayer::AudioPlayer(engine::resource::ResourceManager& resource_manager)
		: resource_manager_(resource_manager) {
	}

	void AudioPlayer::setMasterVolume(float volume) {
		master_volume_ = clamp01(volume);
		resource_manager_.setMasterGain(master_volume_);
	}

	void AudioPlayer::setSoundVolume(float volume) {
		sound_volume_ = clamp01(volume);
		resource_manager_.setSoundGain(master_volume_ * sound_volume_);
	}

	void AudioPlayer::setMusicVolume(float volume) {
		music_volume_ = clamp01(volume);
		resource_manager_.setMusicGain(master_volume_ * music_volume_);
	}

	int AudioPlayer::playSound(const std::string& path) {
		// Current ResourceManager/AudioManager does not expose per-sound gain.
		// Keep layering contract; volume knobs are stored for future mixer gain wiring.
		resource_manager_.playSound(path);
		return 0;
	}

	int AudioPlayer::playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) {
		if (max_distance <= 0.0f) {
			return playSound(path);
		}

		const glm::vec2 delta = emitter_world_pos - listener_world_pos;
		const float dist2 = (delta.x * delta.x) + (delta.y * delta.y);
		const float max2 = max_distance * max_distance;
		if (dist2 > max2) {
			return 0;
		}

		// Simple linear attenuation based on distance.
		const float dist = std::sqrt(dist2);
		const float t = 1.0f - (dist / max_distance);
		const float gain = std::clamp(t, 0.0f, 1.0f);

		// Current AudioManager exposes only tag-level gains, so approximate by temporarily scaling sound gain.
		// This is a best-effort approach until per-track/per-play gains are exposed.
		const float base_gain = master_volume_ * sound_volume_;
		resource_manager_.setSoundGain(base_gain * gain);
		resource_manager_.playSound(path);
		resource_manager_.setSoundGain(base_gain);
		return 0;
	}

	bool AudioPlayer::playMusic(const std::string& path, int loops) {
		(void)loops;
		current_music_ = path;
		resource_manager_.playMusic(path);
		resource_manager_.setMusicGain(master_volume_ * music_volume_);
		return true;
	}

	void AudioPlayer::stopMusic() {
		resource_manager_.stopMusic();
		current_music_.clear();
	}
}
