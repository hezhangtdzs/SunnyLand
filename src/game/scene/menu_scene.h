#pragma once
#include "../../engine/scene/scene.h"
namespace game::data {
    class SessionData;
}
namespace engine::core
{
    class Context;
}
namespace engine::scene
{
    class SceneManager;
}

namespace game::scene
{
    class MenuScene : public engine::scene::Scene
    {
        std::shared_ptr<game::data::SessionData> session_data_;
    public:
        MenuScene(engine::core::Context& context,
                  engine::scene::SceneManager& scene_manager,
                  std::shared_ptr<game::data::SessionData> session_data_);
        ~MenuScene() override = default;

        void init() override;
        bool handleInput() override;
    private:
            // 用于创建UI和按钮回调的私有辅助方法
        void createUI();
        void onResumeClicked();
        void onSaveClicked();
        void onBackClicked();
        void onQuitClicked();
    };
}