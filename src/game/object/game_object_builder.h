#pragma once
#include <memory>
#include <string>
#include <optional>
#include "../../engine/object/object_builder.h"

namespace engine::core {
    class Context;
}
namespace engine::scene {
    class LevelLoader;
}
namespace engine::object {
    class GameObject;
}

namespace game::object {

    /**
     * @class GameObjectBuilder
     * @brief 游戏对象具体生成器
     *
     * 生成器模式(Builder Pattern)的具体实现。
     * 角色：ConcreteBuilder（具体生成器）
     * 职责：实现游戏特定对象的构建步骤，扩展基础ObjectBuilder的功能
     *
     * 本类添加了游戏特定的组件构建逻辑：
     * - 敌人AI行为（巡逻、跳跃、上下移动）
     * - 玩家组件（PlayerComponent）
     * - 道具组件（动画播放）
     *
     * 使用示例：
     * @code
     * // 示例1：从头创建新对象
     * GameObjectBuilder builder(level_loader, context);
     * auto enemy = builder
     *     .configure(&object_json, &tile_json, tile_info)
     *     .setEnemyType("eagle")
     *     .build()
     *     .getGameObject();
     *
     * // 示例2：增强已有对象
     * builder.autoDetectType("frog")
     *        .enhance(existing_object)
     *        .buildEnhancement();
     * @endcode
     */
    class GameObjectBuilder final : public engine::object::ObjectBuilder {
    private:
        // ========== 游戏特定配置 ==========
        std::optional<std::string> enemy_type_;      ///< 敌人类型 (eagle/frog/opossum)
        std::optional<std::string> item_type_;       ///< 道具类型 (fruit/gem)
        bool is_player_ = false;                     ///< 是否为玩家对象

        // ========== 增强模式配置 ==========
        engine::object::GameObject* target_object_ = nullptr;  ///< 目标对象（用于增强已有对象）

    public:
        /**
         * @brief 构造函数
         * @param level_loader 关卡加载器引用
         * @param context 引擎上下文引用
         */
        GameObjectBuilder(engine::scene::LevelLoader& level_loader,
                          engine::core::Context& context);

        /**
         * @brief 虚析构函数
         */
        ~GameObjectBuilder() override = default;

        // 禁止拷贝和移动，保证唯一所有权
        GameObjectBuilder(const GameObjectBuilder&) = delete;
        GameObjectBuilder& operator=(const GameObjectBuilder&) = delete;
        GameObjectBuilder(GameObjectBuilder&&) = delete;
        GameObjectBuilder& operator=(GameObjectBuilder&&) = delete;

        // ========== 核心构建方法 ==========

        /**
         * @brief 构建完整的游戏对象
         *
         * 构建流程：
         * 1. 重置生成器状态
         * 2. 调用父类build()构建基础组件
         * 3. 调用buildGameSpecific()构建游戏特定组件
         *
         * @note 调用此方法前必须先调用 configure 进行配置
         */
        void build() override;

        // ========== 增强模式接口 ==========

        /**
         * @brief 设置要增强的目标对象
         *
         * 这是生成器模式的扩展用法，用于为已存在的对象添加游戏特定组件。
         * 与从头构建不同，增强模式不会创建新对象，而是修改已有对象。
         *
         * @param game_object 已存在的游戏对象指针
         * @return this指针，支持链式调用
         */
        GameObjectBuilder* enhance(engine::object::GameObject* game_object);

        /**
         * @brief 执行增强操作
         *
         * 根据之前配置的类型，为目标对象添加相应的游戏特定组件。
         * 执行完成后会自动重置生成器状态。
         *
         * @return 是否成功增强
         */
        bool buildEnhancement();

        // ========== 配置接口（链式调用） ==========

        /**
         * @brief 设置敌人类型
         * @param type 敌人类型名称 ("eagle", "frog", "opossum")
         * @return this指针，支持链式调用
         */
        GameObjectBuilder* setEnemyType(const std::string& type);

        /**
         * @brief 设置道具类型
         * @param type 道具类型名称 ("fruit", "gem")
         * @return this指针，支持链式调用
         */
        GameObjectBuilder* setItemType(const std::string& type);

        /**
         * @brief 标记为玩家对象
         * @return this指针，支持链式调用
         */
        GameObjectBuilder* setAsPlayer();

        /**
         * @brief 从对象自动推断类型
         *
         * 根据对象名称和标签自动设置相应的类型配置：
         * - "eagle" -> enemy_type = "eagle"
         * - "frog" -> enemy_type = "frog"
         * - "opossum" -> enemy_type = "opossum"
         * - tag == "player" -> is_player = true（支持多个玩家对象如 player, player2）
         * - "fruit" -> item_type = "fruit"
         * - "gem" -> item_type = "gem"
         *
         * @param game_object 游戏对象指针
         * @return this指针，支持链式调用
         */
        GameObjectBuilder* autoDetectType(engine::object::GameObject* game_object);

        /**
         * @brief 重置生成器状态
         *
         * 重置所有配置和状态，使生成器可以重新使用。
         * 通常在以下情况调用：
         * - 开始构建新对象前
         * - 构建流程出错需要重新开始
         *
         * @return this指针，支持链式调用
         */
        GameObjectBuilder* resetBuilder();

    private:
        // ========== 游戏特定构建步骤 ==========

        /**
         * @brief 构建游戏特定组件
         *
         * 根据当前配置，调用相应的构建方法：
         * - 如果有 enemy_type，调用 buildEnemyAI()
         * - 如果 is_player 为 true，调用 buildPlayerComponent()
         * - 如果有 item_type，调用 buildItemComponents()
         *
         * @param game_object 目标游戏对象
         */
        void buildGameSpecific(engine::object::GameObject* game_object);

        /**
         * @brief 为敌人添加AI行为组件
         *
         * 根据 enemy_type_ 创建相应的AI行为：
         * - "eagle": UpDownBehavior（上下飞行）
         * - "frog": JumpBehavior（跳跃）
         * - "opossum": PatrolBehavior（巡逻）
         *
         * @param game_object 目标游戏对象
         */
        void buildEnemyAI(engine::object::GameObject* game_object);

        /**
         * @brief 为玩家添加PlayerComponent
         *
         * 添加 PlayerComponent 并设置 "player" 标签。
         *
         * @param game_object 目标游戏对象
         */
        void buildPlayerComponent(engine::object::GameObject* game_object);

        /**
         * @brief 为道具设置标签和动画
         *
         * 设置 "item" 标签并播放 idle 动画。
         *
         * @param game_object 目标游戏对象
         */
        void buildItemComponents(engine::object::GameObject* game_object);

        /**
         * @brief 重置游戏特定配置
         *
         * 重置 enemy_type_, item_type_, is_player_ 等游戏特定配置。
         * 由 resetBuilder() 和 buildEnhancement() 调用。
         */
        void resetGameBuilder();
    };

} // namespace game::object
