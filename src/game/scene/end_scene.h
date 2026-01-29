#pragma once
/**
 * @file end_scene.h
 * @brief 定义 EndScene 类，用于显示游戏结束画面
 */

#include "../../engine/scene/scene.h"
#include "../data/session_data.h"

namespace game::scene {

/**
 * @class EndScene
 * @brief 游戏结束场景类
 * 
 * 负责显示游戏胜利或失败的画面，包含得分信息和操作按钮
 */
class EndScene final : public engine::scene::Scene {
private:
    /// 会话数据
    std::shared_ptr<game::data::SessionData> session_data_;

public:
    /**
     * @brief 构造函数
     * @param context 引擎上下文
     * @param scene_manager 场景管理器
     * @param session_data 会话数据
     */
    EndScene(engine::core::Context& context, 
             engine::scene::SceneManager& scene_manager, 
             std::shared_ptr<game::data::SessionData> session_data);

    /**
     * @brief 析构函数
     */
    ~EndScene() override;

    /**
     * @brief 初始化场景
     */
    void init() override;

    /**
     * @brief 处理输入事件
     * @return 是否处理了输入事件
     */
    bool handleInput() override;

private:
    /**
     * @brief 创建 UI 元素
     */
    void createUI();

    /**
     * @brief 处理重新开始按钮点击
     */
    void onRestartClicked();

    /**
     * @brief 处理返回主菜单按钮点击
     */
    void onBackClicked();
};

} // namespace game::scene
