#include "time.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_timer.h>

engine::core::Time::Time()
{
	last_time_ =  SDL_GetTicksNS();
	frame_start_time_ = last_time_;
	spdlog::trace("Time 初始化。Last time : {}", last_time_);
}

void engine::core::Time::update()
{
	frame_start_time_ = SDL_GetTicksNS();
	auto current_delta_time = static_cast<double>(frame_start_time_ - last_time_) / 1e9;
	if (target_fps_ > 0)
	{
		limitFrameRate(current_delta_time);
	}
	else {
		delta_time_ = current_delta_time;
	}
	last_time_ = SDL_GetTicksNS();

}

void engine::core::Time::setTimeScale(double scale)
{
	if (scale <= 0.0) {
		spdlog::warn("Time Scale 必须为正数。Setting to 1.0.");
		time_scale_ = 1.0;
	}
	else {
		time_scale_ = scale;
	}
}

void engine::core::Time::setTargetFPS(int fps)
{
	if (fps < 0) {
		spdlog::warn("Target FPS 不能为负。Setting to 0 (unlimited).");
		target_fps_ = 0;
	}
	else {
		target_fps_ = fps;
	}

	if (target_fps_ > 0) {
		target_frame_time_ = 1.0 / static_cast<double>(target_fps_);
		spdlog::info("Target FPS 设置为: {} (Frame time: {:.6f}s)", target_fps_, target_frame_time_);
	}
	else {
		target_frame_time_ = 0.0;
		spdlog::info("Target FPS 设置为: Unlimited");
	}
}

void engine::core::Time::limitFrameRate(float current_delta_time)
{
	if (current_delta_time < target_frame_time_) {
		double time_to_wait = target_frame_time_ - current_delta_time;
		Uint64 ns_to_wait = static_cast<Uint64>(time_to_wait * 1e9);
		SDL_DelayNS(ns_to_wait);
		delta_time_ = static_cast<double>(SDL_GetTicksNS() - last_time_) / 1e9;
	}else{
		delta_time_ = current_delta_time;
	}
}
