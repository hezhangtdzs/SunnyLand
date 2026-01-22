#pragma once
#include "../../engine/scene/scene.h"
#include <memory>

// 前置声明
namespace engine::object {
    class GameObject;
}

namespace game::scene {

    /**
     * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
     */
    class GameScene final : public engine::scene::Scene {
		engine::object::GameObject* player_{ nullptr }; ///< 测试用的游戏对象指针
    public:
        GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager);

        // 覆盖场景基类的核心方法
        void init() override;
        void update(float delta_time) override;
        void render() override;
        void handleInput() override;
        void clean() override;

    private:
        [[nodiscard]] bool initLevel();               ///< @brief 初始化关卡
        [[nodiscard]] bool initPlayer();              ///< @brief 初始化玩家
		[[nodiscard]] bool initEnemyAndItem();		///< @brief 初始化敌人和道具
    };

} // namespace game::scene