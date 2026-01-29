#include "end_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/core/game_state.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_text.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/scene/scene_manager.h"
#include "title_scene.h"
#include "game_scene.h"
#include <spdlog/spdlog.h>

namespace game::scene {

EndScene::EndScene(engine::core::Context& context, 
                   engine::scene::SceneManager& scene_manager, 
                   std::shared_ptr<game::data::SessionData> session_data)
    : Scene("EndScene", context, scene_manager),
      session_data_(std::move(session_data)) {
    if (!session_data_) {
        spdlog::warn("EndScene 构造函数: session_data_ 为空，尝试从 SceneManager 获取...");
        session_data_ = scene_manager_.getSessionData();
    }
    
    if (!session_data_) {
        spdlog::error("EndScene 构造函数: session_data_ 依然为空！");
    } else {
        spdlog::trace("EndScene 构造函数: session_data_ 初始化成功");
    }
}

EndScene::~EndScene() = default;

void EndScene::init() {
    Scene::init();
    createUI();
    spdlog::trace("EndScene 初始化完成");
}

bool EndScene::handleInput() {
    Scene::handleInput();
    return true;
}

void EndScene::createUI() {
    auto window_size = context_.getGameState().getWindowLogicalSize();
    spdlog::debug("EndScene 窗口逻辑尺寸: {} x {}", window_size.x, window_size.y);

    if (!ui_manager_->init(window_size)) {
        spdlog::error("EndScene 中初始化 UIManager 失败!");
        return;
    }

    // 获取游戏胜利/失败状态
    bool is_win = session_data_->getIsWin();
    spdlog::debug("EndScene 游戏状态: {}", is_win ? "胜利" : "失败");

    // 根据游戏结果显示不同的标题
    std::string title_text = is_win ? "YOU WIN!" : "YOU DIED!";
    engine::utils::FColor title_color = is_win ? 
        engine::utils::FColor{0.0f, 1.0f, 0.0f, 1.0f} : // 绿色
        engine::utils::FColor{1.0f, 0.0f, 0.0f, 1.0f}; // 红色

    // 创建标题标签
    auto title_label = std::make_unique<engine::ui::UIText>(context_, 
                                                          title_text, 
                                                          "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                          48);
    title_label->setColor(title_color);
    auto title_size = title_label->getSize();
    spdlog::debug("EndScene 标题大小: {} x {}", title_size.x, title_size.y);
    auto title_x = (window_size.x - title_size.x) / 2.0f;
    auto title_y = window_size.y * 0.15f;
    spdlog::debug("EndScene 标题位置: ({}, {})", title_x, title_y);
    title_label->setPosition(glm::vec2(title_x, title_y));
    ui_manager_->addElement(std::move(title_label));

    // 创建得分信息
    int current_score = session_data_->getCurrentScore();
    int high_score_level1 = session_data_->getHighScoreLevel1();
    int high_score_level2 = session_data_->getHighScoreLevel2();

    // 当前得分
    auto score_text = std::make_unique<engine::ui::UIText>(context_, 
                                                         "Score: " + std::to_string(current_score), 
                                                         "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                         24);
    auto score_size = score_text->getSize();
    auto score_x = (window_size.x - score_size.x) / 2.0f;
    auto score_y = title_y + title_size.y + 30.0f;
    score_text->setPosition(glm::vec2(score_x, score_y));
    ui_manager_->addElement(std::move(score_text));

    // 最高分
    auto high_score_text = std::make_unique<engine::ui::UIText>(context_, 
                                                              "High Scores:", 
                                                              "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                              20);
    auto high_score_size = high_score_text->getSize();
    auto high_score_x = (window_size.x - high_score_size.x) / 2.0f;
    auto high_score_y = score_y + score_size.y + 15.0f;
    high_score_text->setPosition(glm::vec2(high_score_x, high_score_y));
    ui_manager_->addElement(std::move(high_score_text));

    // 第一关最高分
    auto high_score_level1_text = std::make_unique<engine::ui::UIText>(context_, 
                                                                     "Level 1: " + std::to_string(high_score_level1), 
                                                                     "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                                     18);
    auto high_score_level1_size = high_score_level1_text->getSize();
    auto high_score_level1_x = (window_size.x - high_score_level1_size.x) / 2.0f;
    auto high_score_level1_y = high_score_y + high_score_size.y + 5.0f;
    high_score_level1_text->setPosition(glm::vec2(high_score_level1_x, high_score_level1_y));
    ui_manager_->addElement(std::move(high_score_level1_text));

    // 第二关最高分（减去第一关分数）
    int level2_score = high_score_level2 - high_score_level1;
    if (level2_score < 0) {
        level2_score = 0;
    }
    auto high_score_level2_text = std::make_unique<engine::ui::UIText>(context_, 
                                                                     "Level 2: " + std::to_string(level2_score), 
                                                                     "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                                     18);
    auto high_score_level2_size = high_score_level2_text->getSize();
    auto high_score_level2_x = (window_size.x - high_score_level2_size.x) / 2.0f;
    auto high_score_level2_y = high_score_level1_y + high_score_level1_size.y + 5.0f;
    high_score_level2_text->setPosition(glm::vec2(high_score_level2_x, high_score_level2_y));
    ui_manager_->addElement(std::move(high_score_level2_text));

    // --- 创建按钮 --- (2个按钮，设定好大小、间距)
    float button_width = 96.0f; // 按钮稍微小一点
    float button_height = 32.0f;
    float button_spacing = 10.0f;
    float start_y = high_score_level2_y + high_score_level2_size.y + 35.0f; 
    float button_x = (window_size.x - (button_width * 2 + button_spacing)) / 2.0f; // 水平居中
    spdlog::debug("按钮位置 - X: {}, 起始 Y: {}", button_x, start_y);

    // 重新开始按钮
    auto restart_button = std::make_unique<engine::ui::UIButton>(context_,
                                                                "assets/textures/UI/buttons/Restart1.png", 
                                                                "assets/textures/UI/buttons/Restart2.png", 
                                                                "assets/textures/UI/buttons/Restart3.png",
                                                                glm::vec2{button_x, start_y},
                                                                glm::vec2{button_width, button_height},
                                                                [this]() { this->onRestartClicked(); });
    ui_manager_->addElement(std::move(restart_button));

    // 返回主菜单按钮
    auto back_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Back1.png", 
                                                              "assets/textures/UI/buttons/Back2.png", 
                                                              "assets/textures/UI/buttons/Back3.png",
                                                              glm::vec2{button_x + button_width + button_spacing, start_y},
                                                              glm::vec2{button_width, button_height},
                                                              [this]() { this->onBackClicked(); });
    ui_manager_->addElement(std::move(back_button));
}

void EndScene::onRestartClicked() {
    spdlog::debug("EndScene: 点击了重新开始按钮");
    
    // 重置游戏数据
    if (session_data_) {
        session_data_->reset();
        session_data_->save();
    }
    
    // 创建新的游戏场景
    auto game_scene = std::make_unique<GameScene>(
        "GameScene", 
        context_, 
        scene_manager_, 
        session_data_,
        session_data_->getMapPath());
    scene_manager_.requestReplaceScene(std::move(game_scene));
}

void EndScene::onBackClicked() {
    spdlog::debug("EndScene: 点击了返回主菜单按钮");
    
    // 创建标题场景
    auto title_scene = std::make_unique<TitleScene>(
        context_, 
        scene_manager_, 
        session_data_);
    scene_manager_.requestReplaceScene(std::move(title_scene));
}

} // namespace game::scene
