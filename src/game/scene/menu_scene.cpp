#include "menu_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/render/camera.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_text.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/utils/math.h"
#include "../data/session_data.h"
#include "game_scene.h"
#include "title_scene.h"
#include "helps_scene.h"
#include <spdlog/spdlog.h>

namespace game::scene
{
     MenuScene::MenuScene(engine::core::Context& context,   
                  engine::scene::SceneManager& scene_manager,
                  std::shared_ptr<game::data::SessionData> session_data)
        : engine::scene::Scene("MenuScene", context, scene_manager),
          session_data_(std::move(session_data))
    {
        // 如果传入的 session_data 为空，尝试从 scene_manager 获取
        if (!session_data_) {
            spdlog::warn("MenuScene 构造函数: 传入的 session_data 为空，正在从 SceneManager 获取...");
            session_data_ = scene_manager_.getSessionData();
        }
        
        if (!session_data_) {
            spdlog::error("MenuScene 构造函数: session_data_ 依然为空！");
        } else {
            spdlog::trace("MenuScene 构造函数: session_data_ 初始化成功");
        }
    }

    void MenuScene::init()
    {
        context_.getGameState().setState(engine::core::GameStateType::Paused);

        createUI();

        Scene::init();
        spdlog::trace("menuScene 初始化完成");
    }

    bool MenuScene::handleInput()
    {
        Scene::handleInput();
        if (context_.getInputManager().isActionPressed("pause")) {
        spdlog::debug("在菜单场景中按下暂停键，正在恢复游戏...");
        scene_manager_.requestPopScene();       // 弹出自身以恢复底层的GameScene
        context_.getGameState().setState(engine::core::GameStateType::Playing); 
        return false;
    }
        return true;
    }

    void MenuScene::createUI()
    {
        auto window_size = context_.getGameState().getWindowLogicalSize();
        spdlog::debug("MenuScene 窗口逻辑尺寸: {} x {}", window_size.x, window_size.y);
    if (!ui_manager_->init(window_size)) {
        spdlog::error("MenuScene 中初始化 UIManager 失败!");
        return;
    }

    // "PAUSE"标签
    auto pause_label = std::make_unique<engine::ui::UIText>(context_, 
                                                             "PAUSE", 
                                                             "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                             32);
    // 放在中间靠上的位置 
    auto size = pause_label->getSize();
    spdlog::debug("Pause 标签大小: {} x {}", size.x, size.y);
    auto label_y = window_size.y * 0.2f;
    auto label_x = (window_size.x - size.x) / 2.0f;
    spdlog::debug("Pause 标签位置: ({}, {})", label_x, label_y);
    pause_label->setPosition(glm::vec2(label_x, label_y));
    ui_manager_->addElement(std::move(pause_label));

    // --- 创建按钮 --- (4个按钮，设定好大小、间距)
    float button_width = 96.0f; // 按钮稍微小一点
    float button_height = 32.0f;
    float button_spacing = 10.0f;
    float start_y = label_y + 80.0f; // 从标签下方开始，增加间距
    float button_x = (window_size.x - button_width) / 2.0f; // 水平居中
    spdlog::debug("按钮位置 - X: {}, 起始 Y: {}", button_x, start_y);

    // Resume Button
    auto resume_button = std::make_unique<engine::ui::UIButton>(context_,
                                                                "assets/textures/UI/buttons/Resume1.png", 
                                                                "assets/textures/UI/buttons/Resume2.png",  
                                                                "assets/textures/UI/buttons/Resume3.png",
                                                                glm::vec2{button_x, start_y},
                                                                glm::vec2{button_width, button_height},
                                                                [this]() { this->onResumeClicked(); });
    ui_manager_->addElement(std::move(resume_button));

    // Save Button
    start_y += button_height + button_spacing;
    auto save_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Save1.png", 
                                                              "assets/textures/UI/buttons/Save2.png", 
                                                              "assets/textures/UI/buttons/Save3.png",
                                                              glm::vec2{button_x, start_y},
                                                              glm::vec2{button_width, button_height},
                                                              [this]() { this->onSaveClicked(); });
    ui_manager_->addElement(std::move(save_button));

    // Back Button
    start_y += button_height + button_spacing;
    auto back_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Back1.png", 
                                                              "assets/textures/UI/buttons/Back2.png", 
                                                              "assets/textures/UI/buttons/Back3.png",
                                                              glm::vec2{button_x, start_y},
                                                              glm::vec2{button_width, button_height},
                                                              [this]() { this->onBackClicked(); });
    ui_manager_->addElement(std::move(back_button));

    // Quit Button
    start_y += button_height + button_spacing;
    auto quit_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Quit1.png", 
                                                              "assets/textures/UI/buttons/Quit2.png", 
                                                              "assets/textures/UI/buttons/Quit3.png",
                                                              glm::vec2{button_x, start_y},
                                                              glm::vec2{button_width, button_height},
                                                              [this]() { this->onQuitClicked(); });
    ui_manager_->addElement(std::move(quit_button));
    }

    void MenuScene::onResumeClicked()
    {
        spdlog::debug("MenuScene: 点击了 Resume 按钮");
        scene_manager_.requestPopScene();       // 弹出自身以恢复底层的GameScene
        context_.getGameState().setState(engine::core::GameStateType::Playing); 
    }
    void MenuScene::onSaveClicked()
    {
        spdlog::debug("MenuScene: 点击了 Save 按钮");
        if (session_data_) {
            session_data_->save();
        } else {
            spdlog::error("MenuScene: 无法保存，session_data_ 为空");
        }
    }
    void MenuScene::onBackClicked()
    {
        spdlog::debug("MenuScene: 点击了 Back 按钮");
        scene_manager_.requestPopScene();       // 弹出自身以返回TitleScene
        scene_manager_.requestReplaceScene(std::make_unique<TitleScene>(context_, scene_manager_, session_data_));
    }
    void MenuScene::onQuitClicked()
    {
        spdlog::debug("MenuScene: 点击了 Quit 按钮");
        context_.getInputManager().setShouldQuit(true);
    }
}
