#pragma once
#include "../../engine/scene/scene.h"
#include <glm/glm.hpp>
#include <memory>

// 前置声明
namespace engine::object {
    class GameObject;
}

namespace engine::ui {
    class UIPanel;
    class UIImage;
    class UIText;
}

namespace game::data {
    class SessionData;
}

namespace game::scene {

    /**
     * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
     */
    class GameScene final : public engine::scene::Scene {
		engine::object::GameObject* player_{ nullptr }; ///< 测试用的游戏对象指针
        std::string level_path_;                         ///< 当前关卡的文件路径
        std::shared_ptr<game::data::SessionData> session_data_; ///< 共享游戏数据
        
        // HUD相关成员变量
        engine::ui::UIPanel* hud_panel_ = nullptr;       ///< HUD面板
        std::vector<engine::ui::UIImage*> health_icons_; ///< 生命值图标数组
        engine::ui::UIText* score_text_ = nullptr;       ///< 得分文本
    public:
        GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager, std::string level_path = "assets/maps/level1.tmj");
        GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager, std::shared_ptr<game::data::SessionData> session_data, std::string level_path = "assets/maps/level1.tmj");

        // 覆盖场景基类的核心方法
        void init() override;
        void update(float delta_time) override;
        void render() override;
        bool handleInput() override;
        void clean() override;

    private:
        [[nodiscard]] bool initLevel();               ///< @brief 初始化关卡
        [[nodiscard]] bool initPlayer();              ///< @brief 初始化玩家
		[[nodiscard]] bool initEnemyAndItem();		///< @brief 初始化敌人和道具
        void initHUD();                              ///< @brief 初始化HUD界面
        void updateHUD();                            ///< @brief 更新HUD显示

        void handleObjectCollisions();              ///< @brief 处理游戏对象间的碰撞逻辑（从PhysicsEngine获取信息）
        void PlayerVSEnemyCollision(engine::object::GameObject* player, engine::object::GameObject* enemy);  ///< @brief 玩家与敌人碰撞处理
        void PlayerVSItemCollision(engine::object::GameObject* player, engine::object::GameObject* item);    ///< @brief 玩家与道具碰撞处理
        void handleTileTriggers();					 ///< @brief 处理游戏对象与瓦片触发事件的逻辑
        void processHazardDamage(engine::object::GameObject* player); ///< @brief 处理玩家受到的危险伤害 (尖刺、陷阱等)
        void exampleUsageOfGameObjectBuilder();     ///< @brief GameObjectBuilder使用示例（生成器模式）
        /**
         * @brief 创建一个特效对象（一次性）。

         * @param center_pos 特效中心位置
         * @param tag 特效标签（决定特效类型,例如"enemy","item"）
         */
        void createEffect(const glm::vec2& center_pos, const std::string& tag);
        
    };

} // namespace game::scene