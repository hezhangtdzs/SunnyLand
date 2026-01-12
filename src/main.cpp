#include "engine/core/game_app.h"
#include<spdlog/spdlog.h>
int main(int /* argc */, char* /* argv */[]) {
    engine::core::GameApp app;
    spdlog::set_level(spdlog::level::trace);
    app.run();
    return 0;
}