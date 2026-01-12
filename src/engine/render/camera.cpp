#include "camera.h"

	namespace engine::render {
		/**
		 * @brief 析构函数。
		 */
		Camera::~Camera() = default;

		/**
		 * @brief 更新相机每一帧的状态。
		 * @param delta_time 自上一帧以来的时间间隔（秒）。
		 */
		void Camera::update(float delta_time) {
	
		}

		/**
		 * @brief 移动相机。
		 * @param offset 移动的偏移向量。
		 */
		void Camera::move(const glm::vec2& offset) {
			position_ += offset;
			clampPosition();
		}

		/**
		 * @brief 将世界坐标转换为屏幕坐标。
		 * @param world_pos 世界坐标系中的位置。
		 * @return 转换后的屏幕坐标。
		 */
		glm::vec2 Camera::worldToScreen(const glm::vec2& world_pos) const {
			return world_pos - position_;
		}

		/**
		 * @brief 将世界坐标转换为带有视差效果的屏幕坐标。
		 * @param world_pos 世界坐标系中的位置。
		 * @param scroll_factor 滚动因子（视差强度）。
		 * @return 转换后的屏幕坐标。
		 */
		glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2& world_pos, float scroll_factor) const {
			return world_pos - position_ * scroll_factor;
		}

		/**
		 * @brief 将带视差的世界坐标映射到屏幕坐标 (X/Y 独立滚动系数)。
		 * @param world_pos 世界坐标系中的位置。
		 * @param scroll_factor 滚动因子（视差强度）。
		 * @return 转换后的屏幕坐标。
		 */
		glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const {
			return world_pos - position_ * scroll_factor;
		}

		/**
		 * @brief 将屏幕坐标转换为世界坐标。
		 * @param screen_pos 屏幕坐标系中的位置。
		 * @return 转换后的世界坐标。
		 */
		glm::vec2 Camera::screenToWorld(const glm::vec2& screen_pos) const {
			return screen_pos + position_;
		}

		/**
		 * @brief 设置相机的位置。
		 * @param position 新的位置坐标。
		 */
		void Camera::setPosition(const glm::vec2& position) {
			position_ = position;
			clampPosition();
		}

		/**
		 * @brief 设置相机的移动限制边界。
		 * @param bounds 相机允许活动的矩形区域。
		 */
		void Camera::setLimitBounds(const engine::utils::Rect& bounds) {
			limit_bounds_ = bounds;
			clampPosition();
		}

		/**
		 * @brief 获取相机位置。
		 * @return const glm::vec2& 相机中心坐标。
		 */
		const glm::vec2& Camera::getPosition() const {
			return position_;
		}

		/**
		 * @brief 获取相机的限制边界。
		 * @return std::optional<engine::utils::Rect> 配置的边界。
		 */
		std::optional<engine::utils::Rect> Camera::getLimitBounds() const {
			return limit_bounds_;
		}

		/**
		 * @brief 获取相机的视口大小。
		 * @return glm::vec2 视口宽高。
		 */
		glm::vec2 Camera::getViewportSize() const {
			return viewport_size_;
		}

		/**
		 * @brief 修正相机位置，确保其保持在限制边界内。
		 * 逻辑会考虑视口大小，防止相机超出背景区域。
		 */
		void Camera::clampPosition() {
			if (limit_bounds_.has_value() && limit_bounds_->size.x > 0 && limit_bounds_->size.y > 0) {
				glm::vec2 min_cam_pos = limit_bounds_->position;
				glm::vec2 max_cam_pos = limit_bounds_->position + limit_bounds_->size - viewport_size_;
				
				// 确保最小边界不大于最大边界
				min_cam_pos = glm::min(min_cam_pos, max_cam_pos);
				max_cam_pos = glm::max(min_cam_pos, max_cam_pos);
				
				position_ = glm::clamp(position_, min_cam_pos, max_cam_pos);
			}
		}
	} // namespace engine::render