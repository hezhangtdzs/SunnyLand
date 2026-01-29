#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace game::data {

/**
 * @class SessionData
 * @brief 管理不同游戏场景之间的共享游戏状态
 * 
 * 负责存储和管理玩家生命值、得分、当前关卡等跨场景的游戏数据
 * 使用 shared_ptr 在场景间共享数据，生命周期独立于场景
 */
class SessionData final {
private:
    int current_health_;
    int max_health_;
    int current_score_;
    int score_confirmed_; ///< 已确认的关卡分数（存档时使用此值）
    int high_score_level1_; ///< 第一关最高分
    int high_score_level2_; ///< 第二关最高分
    std::string map_path_;
    std::string save_file_path_;
    bool is_win_; ///< 游戏胜利/失败状态

    /**
     * @brief 构造函数
     * @param max_health 玩家最大生命值
     * @param initial_map_path 初始关卡路径
     * @param save_file_path 存档文件路径
     */
    SessionData(int max_health, const std::string& initial_map_path, const std::string& save_file_path);

public:
    /**
     * @brief 获取 SessionData 单例实例
     * @param max_health 玩家最大生命值（仅首次调用时有效）
     * @param initial_map_path 初始关卡路径（仅首次调用时有效）
     * @param save_file_path 存档文件路径（仅首次调用时有效）
     * @return std::shared_ptr<SessionData> SessionData 实例
     */
    static std::shared_ptr<SessionData> getInstance(
        int max_health = 3,
        const std::string& initial_map_path = "assets/maps/level1.tmj",
        const std::string& save_file_path = "assets/save_data.json");

    // 禁止拷贝和移动语义
    SessionData(const SessionData&) = delete;
    SessionData& operator=(const SessionData&) = delete;
    SessionData(SessionData&&) = delete;
    SessionData& operator=(SessionData&&) = delete;

    /**
     * @brief 保存游戏数据到文件
     * @return bool 保存是否成功
     */
    bool save() const;

    /**
     * @brief 从文件加载游戏数据
     * @return bool 加载是否成功
     */
    bool load();

    // Getter 和 Setter 方法
    int getCurrentHealth() const { return current_health_; }
    void setCurrentHealth(int health);

    int getMaxHealth() const { return max_health_; }
    void setMaxHealth(int max_health);

    int getCurrentScore() const { return current_score_; }
    void setCurrentScore(int score);
    void addScore(int score);

    const std::string& getMapPath() const { return map_path_; }
    void setMapPath(const std::string& map_path) { map_path_ = map_path; }

    const std::string& getSaveFilePath() const { return save_file_path_; }

    /**
     * @brief 获取第一关最高分
     * @return 第一关最高分
     */
    int getHighScoreLevel1() const { return high_score_level1_; }

    /**
     * @brief 获取第二关最高分
     * @return 第二关最高分
     */
    int getHighScoreLevel2() const { return high_score_level2_; }

    /**
     * @brief 更新最高分
     * @return 是否更新了最高分
     */
    bool updateHighScore();

    /**
     * @brief 转换关卡记录当前分数
     */
    void confirmScore() { score_confirmed_ = current_score_; }

    /**
     * @brief 检查是否回到第一关并清空当前分数
     */
    void checkAndResetScore();

    /**
     * @brief 重置游戏数据到初始状态
     */
    void reset();

    /**
     * @brief 获取游戏胜利/失败状态
     * @return 游戏是否胜利
     */
    bool getIsWin() const { return is_win_; }

    /**
     * @brief 设置游戏胜利/失败状态
     * @param is_win 游戏是否胜利
     */
    void setIsWin(bool is_win) { is_win_ = is_win; }

private:
    // JSON 序列化和反序列化
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);

    // 单例实例
    static std::weak_ptr<SessionData> instance_;
};

} // namespace game::data
