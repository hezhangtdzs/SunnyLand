#include "title_scene.h"
#include "../../engine/core/context.h"
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
#include "helps_scene.h"
#include <spdlog/spdlog.h>

namespace game::scene {

/**
 * @brief 构造函数
 * 
 * @param context 引擎上下文
 * @param scene_manager 场景管理器
 * @param session_data 游戏会话数据
 * @details 初始化标题场景，设置场景名称和会话数据
 */
TitleScene::TitleScene(engine::core::Context& context,
                       engine::scene::SceneManager& scene_manager,
                       std::shared_ptr<game::data::SessionData> session_data)
    : engine::scene::Scene("TitleScene", context, scene_manager),
      session_data_(std::move(session_data))
{
    if (!session_data_) {
        spdlog::warn("TitleScene 没有接收到 SessionData，正在从 SceneManager 或单例获取...");
        session_data_ = scene_manager_.getSessionData();
        if (!session_data_) {
            session_data_ = game::data::SessionData::getInstance();
        }
    }
    spdlog::trace("TitleScene 创建.");
}

/**
 * @brief 初始化场景
 * 
 * @details 加载背景地图，创建UI元素，完成场景初始化
 */
void TitleScene::init() {
    if (is_initialized_) {
        return;
    }
    // 加载背景地图
    engine::scene::LevelLoader level_loader;
    if (!level_loader.loadLevel("assets/maps/level0.tmj", *this)) {
         spdlog::error("加载背景失败");
         return;
    }

    // 创建 UI 元素
    createUI();

    Scene::init();
    spdlog::trace("TitleScene 初始化完成.");
}

/**
 * @brief 创建 UI 界面元素
 * 
 * @details 创建标题图片、按钮面板和 credits 标签，设置按钮回调函数
 */
void TitleScene::createUI() {
    spdlog::trace("创建 TitleScene UI...");
    auto window_size = glm::vec2(640.0f, 360.0f);

    // 设置音量
    context_.getAudioPlayer().setMusicVolume(0.2f);  // 设置背景音乐音量为20%
    context_.getAudioPlayer().setSoundVolume(0.5f);  // 设置音效音量为50%

    // 设置背景音乐
    // context_.getAudioPlayer().playMusic("assets/audio/platformer_level03_loop.ogg");

    // 创建标题图片 (使用路径获取原始大小)
    std::string title_path = "assets/textures/UI/title-screen.png";
    glm::vec2 texture_size = context_.getResourceManager().getTextureSize(title_path);
    auto title_image = std::make_unique<engine::ui::UIImage>(context_, title_path);
    title_image->setSize(texture_size * 2.0f);      // 放大为2倍

    // 水平居中
    auto title_pos = (window_size - title_image->getSize()) / 2.0f - glm::vec2(0.0f, 50.0f);
    title_image->setPosition(title_pos);
    ui_manager_->addElement(std::move(title_image));

    // --- 创建按钮面板并居中 --- (4个按钮，设定好大小、间距)
    float button_width = 96.0f;
    float button_height = 32.0f;
    float button_spacing = 20.0f;
    int num_buttons = 4;

    // 计算面板总宽度
    float panel_width = num_buttons * button_width + (num_buttons - 1) * button_spacing;
    float panel_height = button_height;

    // 计算面板位置使其居中
    float panel_x = (window_size.x - panel_width) / 2.0f;
    float panel_y = window_size.y * 0.65f;  // 垂直位置中间靠下

    auto button_panel = std::make_unique<engine::ui::UIPanel>(context_);
    button_panel->setPosition(glm::vec2(panel_x, panel_y));
    button_panel->setSize(glm::vec2(panel_width, panel_height));

    // --- 创建按钮并添加到 UIPanel (位置是相对于 UIPanel 的 0,0) ---
    glm::vec2 current_button_pos = glm::vec2(0.0f, 0.0f);
    glm::vec2 button_size = glm::vec2(button_width, button_height);

    // Start Button
    auto start_button = std::make_unique<engine::ui::UIButton>(context_, 
                                                              "assets/textures/UI/buttons/Start1.png",
                                                              "assets/textures/UI/buttons/Start2.png",
                                                              "assets/textures/UI/buttons/Start3.png",
                                                              current_button_pos,
                                                              button_size,
                                                              [this]() { this->onStartGameClick();});
    button_panel->addChild(std::move(start_button));

    // Load Button
    current_button_pos.x += button_width + button_spacing;
    auto load_button = std::make_unique<engine::ui::UIButton>(context_, 
                                                              "assets/textures/UI/buttons/Load1.png",
                                                              "assets/textures/UI/buttons/Load2.png",
                                                              "assets/textures/UI/buttons/Load3.png",
                                                              current_button_pos,
                                                              button_size,
                                                              [this]() { this->onLoadGameClick();});
    button_panel->addChild(std::move(load_button));

    // Helps Button
    current_button_pos.x += button_width + button_spacing;
    auto helps_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Helps1.png",
                                                              "assets/textures/UI/buttons/Helps2.png",
                                                              "assets/textures/UI/buttons/Helps3.png",
                                                              current_button_pos,
                                                              button_size,
                                                              [this]() { this->onHelpsClick();});
    button_panel->addChild(std::move(helps_button));

    // Quit Button
    current_button_pos.x += button_width + button_spacing;
    auto quit_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Quit1.png",
                                                              "assets/textures/UI/buttons/Quit2.png",
                                                              "assets/textures/UI/buttons/Quit3.png",
                                                              current_button_pos,
                                                              button_size,
                                                              [this]() { this->onQuitClick();});
    button_panel->addChild(std::move(quit_button));

    // 将 UIPanel 添加到UI管理器
    ui_manager_->addElement(std::move(button_panel));

    // 创建 Credits 标签
    auto credits_text = std::make_unique<engine::ui::UIText>(context_,
                                                               "SunnyLand Credits: XXX - 2025",
                                                               "assets/fonts/VonwaonBitmap-16px.ttf",
                                                               16);
    credits_text->setColor(engine::utils::FColor{ 0.8f, 0.8f, 0.8f, 1.0f });
    credits_text->setPosition(glm::vec2{(window_size.x - credits_text->getSize().x) / 2.0f, 
                                          window_size.y - credits_text->getSize().y - 10.0f});
    ui_manager_->addElement(std::move(credits_text));

    spdlog::trace("TitleScene UI 创建完成.");
}

/**
 * @brief 更新场景逻辑
 * 
 * @param delta_time 时间增量（秒）
 * @details 更新场景逻辑，使相机自动向右移动，营造滚动背景效果
 */
void TitleScene::update(float delta_time) {
    Scene::update(delta_time);

    // 相机自动向右移动
    context_.getCamera().move(glm::vec2(delta_time * 100.0f, 0.0f));
}

/**
 * @brief 开始游戏按钮点击回调
 * 
 * @details 重置会话数据，切换到游戏场景
 */
void TitleScene::onStartGameClick() {
    spdlog::debug("开始游戏按钮被点击。");
    if (session_data_) {
        session_data_->reset();
    }
    scene_manager_.requestReplaceScene(std::make_unique<GameScene>("GameScene", context_, scene_manager_, session_data_, "assets/maps/level1.tmj"));
}

/**
 * @brief 加载游戏按钮点击回调
 * 
 * @details 加载保存的游戏数据，切换到游戏场景
 */
void TitleScene::onLoadGameClick() {
    spdlog::debug("加载游戏按钮被点击。");
    if (!session_data_) {
        spdlog::error("游戏状态为空，无法加载。");
        return;
    }

    if (session_data_->load()) {
        spdlog::debug("保存文件加载成功。开始游戏...");
        scene_manager_.requestReplaceScene(std::make_unique<GameScene>("GameScene", context_, scene_manager_, session_data_, session_data_->getMapPath()));
    } else {
        spdlog::warn("加载保存文件失败。");
    }
}

/**
 * @brief 帮助按钮点击回调
 * 
 * @details 切换到帮助场景
 */
void TitleScene::onHelpsClick() {
    spdlog::debug("帮助按钮被点击。");
    scene_manager_.requestPushScene(std::make_unique<HelpsScene>(context_, scene_manager_));
}

/**
 * @brief 退出按钮点击回调
 * 
 * @details 设置游戏退出标志，关闭游戏
 */
void TitleScene::onQuitClick() {
    spdlog::debug("退出按钮被点击。");
    context_.getInputManager().setShouldQuit(true);
}

} // namespace game::scene