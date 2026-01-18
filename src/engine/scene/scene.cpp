#include "scene.h"
#include<spdlog/spdlog.h>
#include <algorithm> 
#include "../core/context.h"
#include "../physics/physics_engine.h"
#include "../object/game_object.h"

engine::scene::Scene::Scene(const std::string& scene_name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
	:scene_name_(scene_name), context_(context), scene_manager_(scene_manager)
{
	spdlog::info("Scene {} 创建", scene_name_);
}

engine::scene::Scene::~Scene() = default;

void engine::scene::Scene::init()
{
	is_initialized_ = true;
	spdlog::trace("Scene {} 初始化", scene_name_);
}

void engine::scene::Scene::update(float delta_time)
{
	if(!is_initialized_) return;

	// **首先更新物理引擎**
	context_.getPhysicsEngine().update(delta_time);
	for (auto it = game_objects_.begin(); it != game_objects_.end();)
	{
			if (*it && !(*it)->getNeedRemove()){
				(*it)->update(delta_time, context_);
			++it;
			}
			else {
				if (*it) {
					(*it)->clean();
				}
				it = game_objects_.erase(it);
			}
		
	}
	processPendingGameObjects();
}

void engine::scene::Scene::render()
{
	if(is_initialized_){
		for (const auto& obj : game_objects_) {
			if (obj) {
				obj->render(context_);
			}
		}
	}
}

void engine::scene::Scene::handleInput()
{
	if (is_initialized_) {
		for (auto it = game_objects_.begin(); it != game_objects_.end();)
		{
			if (*it && !(*it)->getNeedRemove()) {
				(*it)->handleInput(context_);
				++it;
			}
			else {
				if (*it) {
					(*it)->clean();
				}
				it = game_objects_.erase(it);
			}

		}
	}
}

void engine::scene::Scene::clean()
{
	if(is_initialized_){
		for (auto& obj : game_objects_) {
			if (obj) {
				obj->clean();
			}
		}
		game_objects_.clear();
		is_initialized_ = false;
		spdlog::trace("Scene {} 清理完成", scene_name_);
	}
}

void engine::scene::Scene::addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object)
{
	if (game_object) game_objects_.emplace_back(std::move(game_object));
	spdlog::trace("Scene {} 添加游戏对象，当前对象数量：{}", scene_name_, game_objects_.size());
}

void engine::scene::Scene::safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object)
{
	if (game_object) pending_game_objects_.emplace_back(std::move(game_object));
	spdlog::trace("Scene {} 安全添加游戏对象，待处理对象数量：{}", scene_name_, pending_game_objects_.size());
}

void engine::scene::Scene::removeGameObject(engine::object::GameObject* game_object)
{
	if (!game_object) {
		spdlog::warn("尝试从场景 '{}' 中移除一个空的游戏对象指针。", scene_name_);
		return;
	}
	auto it = std::remove_if(game_objects_.begin(), game_objects_.end(),
				[game_object](const std::unique_ptr<engine::object::GameObject>& obj) {
					return obj.get() == game_object;
				});
	if (it != game_objects_.end()) {
		(*it)->clean();
		game_objects_.erase(it, game_objects_.end());
		spdlog::trace("Scene {} 移除游戏对象，当前对象数量：{}", scene_name_, game_objects_.size());
	}
	else {
		spdlog::warn("在场景 '{}' 中未找到要移除的游戏对象。", scene_name_);
	}
}

void engine::scene::Scene::safeRemoveGameObject(engine::object::GameObject* game_object)
{
	game_object->setNeedRemove(true);
}

engine::object::GameObject* engine::scene::Scene::findGameObjectByName(const std::string& name) const
{
	for (const auto& obj : game_objects_) {
		if (obj->getName() == name) {
			return obj.get();
		}
	}
	return nullptr;
}

void engine::scene::Scene::processPendingGameObjects()
{
	for (auto& obj : pending_game_objects_) {
		addGameObject(std::move(obj));
	}
	pending_game_objects_.clear();

}
