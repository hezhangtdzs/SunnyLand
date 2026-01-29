#include "session_data.h"
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

// 初始化单例实例
std::weak_ptr<game::data::SessionData> game::data::SessionData::instance_;

game::data::SessionData::SessionData(int max_health, const std::string& initial_map_path, const std::string& save_file_path)
    : current_health_(max_health),
      max_health_(max_health),
      current_score_(0),
      score_confirmed_(0),
      high_score_level1_(0),
      high_score_level2_(0),
      map_path_(initial_map_path),
      save_file_path_(save_file_path),
      is_win_(false) {
    spdlog::info("SessionData initialized with max health: {}, initial map: {}", max_health, initial_map_path);
}

std::shared_ptr<game::data::SessionData> game::data::SessionData::getInstance(
    int max_health,
    const std::string& initial_map_path,
    const std::string& save_file_path) {
    auto instance = instance_.lock();
    if (!instance) {
        instance = std::shared_ptr<SessionData>(
            new SessionData(max_health, initial_map_path, save_file_path));
        instance_ = instance;
    }
    return instance;
}

void game::data::SessionData::setCurrentHealth(int health) {
    if (health < 0) {
        current_health_ = 0;
        spdlog::info("Player health set to 0 (dead)");
    } else if (health > max_health_) {
        current_health_ = max_health_;
    } else {
        current_health_ = health;
    }
    spdlog::debug("Current health updated: {}/{}", current_health_, max_health_);
}

void game::data::SessionData::setMaxHealth(int max_health) {
    if (max_health <= 0) {
        spdlog::warn("Invalid max health: {}. Setting to default 3.", max_health);
        max_health_ = 3;
    } else {
        max_health_ = max_health;
    }
    
    // 确保当前生命值不超过新的最大生命值
    if (current_health_ > max_health_) {
        current_health_ = max_health_;
    }
    
    spdlog::debug("Max health updated: {}", max_health_);
}

void game::data::SessionData::setCurrentScore(int score) {
    if (score < 0) {
        current_score_ = 0;
    } else {
        current_score_ = score;
    }
    spdlog::debug("Score updated: {}", current_score_);
}

void game::data::SessionData::addScore(int score) {
    if (score > 0) {
        current_score_ += score;
        spdlog::debug("Score added: {}, total: {}", score, current_score_);
        // 更新最高分
        updateHighScore();
    }
}

bool game::data::SessionData::updateHighScore() {
    bool updated = false;
    
    // 根据当前关卡更新对应的最高分
    if (map_path_.find("level1") != std::string::npos) {
        if (current_score_ > high_score_level1_) {
            high_score_level1_ = current_score_;
            updated = true;
            spdlog::info("Level 1 high score updated: {}", high_score_level1_);
        }
    } else if (map_path_.find("level2") != std::string::npos) {
        if (current_score_ > high_score_level2_) {
            high_score_level2_ = current_score_;
            updated = true;
            spdlog::info("Level 2 high score updated: {}", high_score_level2_);
        }
    }
    
    // 如果更新了最高分，保存数据
    if (updated) {
        save();
    }
    
    return updated;
}

void game::data::SessionData::checkAndResetScore() {
    // 检查是否回到第一关，如果是则清空当前分数
    if (map_path_.find("level1") != std::string::npos) {
        if (current_score_ > 0) {
            spdlog::info("Returning to level 1, resetting current score: {}", current_score_);
            current_score_ = 0;
            score_confirmed_ = 0;
        }
    }
}

void game::data::SessionData::reset() {
    current_health_ = max_health_;
    current_score_ = 0;
    score_confirmed_ = 0;
    map_path_ = "assets/maps/level1.tmj";
    is_win_ = false;
    // 保持最高分不变
    spdlog::info("SessionData reset to initial state, high scores preserved");
}

nlohmann::json game::data::SessionData::toJson() const {
nlohmann::json json_data;
json_data["current_health"] = current_health_;
json_data["max_health"] = max_health_;
json_data["current_score"] = score_confirmed_;
json_data["high_score_level1"] = high_score_level1_;
    json_data["high_score_level2"] = high_score_level2_;
    json_data["map_path"] = map_path_;
    json_data["is_win"] = is_win_;
    return json_data;
}

void game::data::SessionData::fromJson(const nlohmann::json& json) {
    if (json.contains("current_health")) {
        current_health_ = json["current_health"];
    }
    if (json.contains("max_health")) {
        max_health_ = json["max_health"];
    }
    if (json.contains("current_score")) {
        current_score_ = json["current_score"];
        score_confirmed_ = current_score_;
    }
    if (json.contains("high_score_level1")) {
        high_score_level1_ = json["high_score_level1"];
    }
    if (json.contains("high_score_level2")) {
        high_score_level2_ = json["high_score_level2"];
    }
    if (json.contains("map_path")) {
        map_path_ = json["map_path"];
    }
    if (json.contains("is_win")) {
        is_win_ = json["is_win"];
    }
    spdlog::info("Loaded session data: {}/{}, score: {}, high scores: {}/{}. map: {}, is_win: {}", 
                 current_health_, max_health_, current_score_, high_score_level1_, high_score_level2_, map_path_, is_win_);
}

bool game::data::SessionData::save() const {
    try {
        std::ofstream file(save_file_path_);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file: {}", save_file_path_);
            return false;
        }
        
        auto json_data = toJson();
        file << json_data.dump(4); // 格式化输出，缩进4个空格
        file.close();
        
        spdlog::info("Game data saved successfully to: {}", save_file_path_);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error saving game data: {}", e.what());
        return false;
    }
}

bool game::data::SessionData::load() {
    try {
        std::ifstream file(save_file_path_);
        if (!file.is_open()) {
            spdlog::warn("Save file not found: {}. Using default values.", save_file_path_);
            return false;
        }
        
        nlohmann::json json_data;
        file >> json_data;
        file.close();
        
        fromJson(json_data);

        // 加载后确保状态是可玩的
        is_win_ = false; 
        if (current_health_ <= 0) {
            current_health_ = max_health_;
            spdlog::info("Loaded health was 0, resetting to max health: {}", max_health_);
        }

        spdlog::info("Game data loaded successfully from: {}", save_file_path_);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error loading game data: {}. Using default values.", e.what());
        return false;
    }
}
