#include "input_manager.h"
#include "spdlog/spdlog.h"
#include "../core/config.h"
engine::input::InputManager::InputManager(SDL_Renderer* sdl_renderer, const engine::core::Config* config)
	:sdl_renderer_(sdl_renderer)
{
	if (!sdl_renderer_) {
		spdlog::error("输入管理器: SDL_Renderer 为空指针");
		throw std::runtime_error("输入管理器: SDL_Renderer 为空指针");
	}
	initializeMapFromConfig(config);

	float x, y;
	SDL_GetMouseState(&x, &y);
	mouse_position_ = { x,y };
	spdlog::trace("初始鼠标位置: ({}, {})", mouse_position_.x, mouse_position_.y);
	
}

void engine::input::InputManager::Update()
{
	for (auto& [state_name, state] : action_states_) {
		if (state == ActionState::PRESSED_THIS_FRAME) {
			state = ActionState::HELD_DOWN;
		}
		else if (state == ActionState::RELEASED_THIS_FRAME) {
			state = ActionState::INACTIVE;
		}
	}
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		processEvent(event);
	}
}

bool engine::input::InputManager::isActionDown(const std::string& action_name) const
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return false;
	}else return it->second == ActionState::HELD_DOWN || it->second == ActionState::PRESSED_THIS_FRAME;
}

bool engine::input::InputManager::isActionPressed(const std::string& action_name) const
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return false;
	}else return it->second == ActionState::PRESSED_THIS_FRAME;

}

bool engine::input::InputManager::isActionReleased(const std::string& action_name) const
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return false;
	}
	else return it->second == ActionState::RELEASED_THIS_FRAME;
}

glm::vec2 engine::input::InputManager::getMousePosition() const
{
	return mouse_position_;
}

bool engine::input::InputManager::shouldQuit() const
{
	return should_quit_;
}

void engine::input::InputManager::setShouldQuit(bool value)
{
	should_quit_ = value;
}

glm::vec2 engine::input::InputManager::getLogicalMousePosition() const
{
	glm::vec2 logical_pos;
	SDL_RenderCoordinatesFromWindow(sdl_renderer_, mouse_position_.x, mouse_position_.y, &logical_pos.x, &logical_pos.y);
	return logical_pos;
}

void engine::input::InputManager::processEvent(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP: {
		SDL_Scancode scancode = event.key.scancode;
		bool is_down = event.key.down;
		bool is_repeat = event.key.repeat;
		if (auto it = input_to_action_.find(scancode); it == input_to_action_.end()) {
			spdlog::warn("输入映射警告: 未找到按键 {} 的映射.", SDL_GetScancodeName(scancode));
			return;
		}
		else {
			const std::vector<std::string>& actions = it->second;
			for (const std::string& action_name : actions) {
				updateActionStates(action_name, is_down, is_repeat);
			}
		}
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP: {
		Uint32 mouse_button = event.button.button;
		bool is_down = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
		if (auto it = input_to_action_.find(mouse_button); it == input_to_action_.end()) {
			spdlog::warn("输入映射警告: 未找到鼠标按钮 {} 的映射.", mouse_button);
			return;
		}
		else {
			const std::vector<std::string>& actions = it->second;
			for (const std::string& action_name : actions) {
				updateActionStates(action_name, is_down, false);
			}
		}
		break;
	}
	case SDL_EVENT_MOUSE_MOTION: {
		mouse_position_ = glm::vec2(event.motion.x, event.motion.y);
		break;
	}
	case SDL_EVENT_QUIT: {
		should_quit_=true;
		break;
	}
	default:
		break;
	}

}

void engine::input::InputManager::initializeMapFromConfig(const engine::core::Config* config)
{
	spdlog::trace("初始化输入映射...");
	actions_to_keyname_ = config->input_mappings_;
	input_to_action_.clear();
	action_states_.clear();

	if (actions_to_keyname_.find("MouseLeftClick") == actions_to_keyname_.end()) {
		spdlog::debug("配置中没有定义 'MouseLeftClick' 动作, 添加默认映射到 'MouseLeft'.");
		actions_to_keyname_["MouseLeftClick"] = { "MouseLeft" };
	}
	if (actions_to_keyname_.find("MouseRightClick") == actions_to_keyname_.end()) {
		spdlog::debug("配置中没有定义 'MouseRightClick' 动作, 添加默认映射到 'MouseRight'.");
		actions_to_keyname_["MouseRightClick"] = { "MouseRight" };
	}

	for (const auto& [action_name, key_names] : actions_to_keyname_) {
		action_states_[action_name] = ActionState::INACTIVE;
		spdlog::trace("映射动作: {}", action_name);

		for (const std::string& key_name : key_names) {
			SDL_Scancode scancode = stringToScancode(key_name);
			Uint32 mouse_button = stringToMouseButton(key_name);
			if (scancode != SDL_SCANCODE_UNKNOWN) {      // 如果scancode有效,则将action添加到scancode_to_actions_map_中
				input_to_action_[scancode].push_back(action_name);
				spdlog::trace("  映射按键: {} (Scancode: {}) 到动作: {}", key_name, static_cast<int>(scancode), action_name);
			}
			else if (mouse_button != 0) {             // 如果鼠标按钮有效,则将action添加到mouse_button_to_actions_map_中
				input_to_action_[mouse_button].push_back(action_name);
				spdlog::trace("  映射鼠标按钮: {} (Button ID: {}) 到动作: {}", key_name, static_cast<int>(mouse_button), action_name);
				// else if: 未来可添加其它输入类型 ...
			}
			else {
				spdlog::warn("输入映射警告: 未知键或按钮名称 '{}' 用于动作 '{}'.", key_name, action_name);
			}
		}
	}
	
}

void engine::input::InputManager::updateActionStates(const std::string& action_name, bool is_input_active, bool is_repeat_event)
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return;
	}
	else {
		if (is_input_active) {
			if (is_repeat_event) {
				it->second = ActionState::HELD_DOWN;
			}
			else {
				it->second = ActionState::PRESSED_THIS_FRAME;
			}
		}
		else {
			it->second = ActionState::RELEASED_THIS_FRAME;
		}
	}

}

SDL_Scancode engine::input::InputManager::stringToScancode(const std::string& key_name) const
{
	return SDL_GetScancodeFromName(key_name.c_str());
}

Uint32 engine::input::InputManager::stringToMouseButton(const std::string& button_name) const
{
	if (button_name == "MouseLeft") return SDL_BUTTON_LEFT;
	if (button_name == "MouseMiddle") return SDL_BUTTON_MIDDLE;
	if (button_name == "MouseRight") return SDL_BUTTON_RIGHT;
	if (button_name == "MouseX1") return SDL_BUTTON_X1;
	if (button_name == "MouseX2") return SDL_BUTTON_X2;
	return 0;
}
