#include "audio_component.h"
#include "../core/context.h"
#include "../audio/audio_player.h"
#include "transform_component.h"
#include "../object/game_object.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <cmath>

namespace engine::component {
	static bool canPlay(const std::string& id, uint64_t min_interval_ms, std::unordered_map<std::string, uint64_t>& last_play_ticks) {
		if (min_interval_ms == 0) return true;
		const uint64_t now = SDL_GetTicks();
		auto it = last_play_ticks.find(id);
		if (it != last_play_ticks.end() && (now - it->second) < min_interval_ms) {
			return false;
		}
		last_play_ticks[id] = now;
		return true;
	}

	void AudioComponent::playSound(const std::string& id, engine::core::Context& context) {
		auto it = action_sounds_.find(id);
		if (it == action_sounds_.end()) {
			spdlog::debug("音效ID '{}' 没有关联的资源路径。", id);
			return;
		}
		if (!canPlay(id, min_interval_ms_, last_play_ticks_)) return;
		context.getAudioPlayer().playSound(it->second);
	}

	void AudioComponent::playSoundSpatial(const std::string& id, engine::core::Context& context, const glm::vec2& listener_world_pos, float max_distance) {
		auto it = action_sounds_.find(id);
		if (it == action_sounds_.end()) {
			spdlog::debug("音效ID '{}' 没有关联的资源路径。", id);
			return;
		}
		if (!canPlay(id, min_interval_ms_, last_play_ticks_)) return;

		glm::vec2 emitter = listener_world_pos;
		if (owner_) {
			if (auto* tc = owner_->getComponent<engine::component::TransformComponent>()) {
				emitter = tc->getPosition();
			}
		}
		const glm::vec2 delta = emitter - listener_world_pos;
		const float dist2 = (delta.x * delta.x) + (delta.y * delta.y);
		const float dist = std::sqrt(dist2);

		spdlog::trace(
			"AudioComponent: spatial play id='{}' path='{}' emitter=({}, {}) listener=({}, {}) dist={} max={}",
			id,
			it->second,
			emitter.x,
			emitter.y,
			listener_world_pos.x,
			listener_world_pos.y,
			dist,
			max_distance
		);
		context.getAudioPlayer().playSoundSpatial(it->second, emitter, listener_world_pos, max_distance);
	}

	void AudioComponent::playSoundNearCamera(const std::string& id, engine::core::Context& context, float max_distance) {
		const auto listener = context.getCamera().getPosition() + context.getCamera().getViewportSize() * 0.5f;
		playSoundSpatial(id, context, listener, max_distance);
	}

	void AudioComponent::playDirect(const std::string& file_path, engine::core::Context& context) {
		context.getAudioPlayer().playSound(file_path);
	}
}
