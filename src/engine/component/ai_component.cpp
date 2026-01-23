#include "ai_component.h"
#include <spdlog/spdlog.h>
#include "../object/game_object.h"

engine::component::AIComponent::AIComponent(std::unique_ptr<AIBehavior> behavior)
    : behavior_(std::move(behavior))
{
    if (!behavior_) {
        spdlog::error("AIComponent 创建失败：传入的 AIBehavior 指针为空");
    } else {
        spdlog::trace("AIComponent 创建完成");
    }
}

void engine::component::AIComponent::setBehavior(std::unique_ptr<AIBehavior> behavior)
{
    if (behavior_) {
        behavior_->clean();
    }
    behavior_ = std::move(behavior);
    if (behavior_ && owner_) {
        behavior_->init(owner_);
    }
    spdlog::trace("AIComponent 行为已更新");
}

engine::component::AIBehavior* engine::component::AIComponent::getBehavior() const
{
    return behavior_.get();
}

void engine::component::AIComponent::init()
{
    if (owner_ && behavior_) {
        behavior_->init(owner_);
        spdlog::trace("AIComponent 初始化完成");
    } else {
        spdlog::error("AIComponent 初始化失败：缺少所有者或行为");
    }
}

void engine::component::AIComponent::update(float deltaTime, engine::core::Context& context)
{
    if (owner_ && behavior_) {
        behavior_->update(owner_, deltaTime, context);
    }
}

void engine::component::AIComponent::clean()
{
    if (behavior_) {
        behavior_->clean();
    }
    spdlog::trace("AIComponent 清理完成");
}
