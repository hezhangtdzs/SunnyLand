#pragma once
#include <SDL3/SDL_rect.h>
#include <vector>
#include <string>
namespace engine::render {
	struct AnimationFrame {
		SDL_FRect src_rect;  // 纹理源矩形
		float duration;  // 持续时间（秒）
	};
	class Animation final {
	private:
		std::string name_;
		std::vector<AnimationFrame> frames_;
		float total_duration_;
		bool loop_;
	public:
		Animation(const std::string& name = "default", bool loop = true);
		~Animation() = default;

		void addFrame(const SDL_FRect& source_rect, float duration);
		const AnimationFrame& getFrame(float time) const;

		// ... Getters and Setters ...
		const std::string& getName() const { return name_; }
		float getTotalDuration() const { return total_duration_; }
		bool isLooping() const { return loop_; }
		bool isEmpty() const { return frames_.empty(); }
	};

}