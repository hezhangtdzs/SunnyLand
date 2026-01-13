# 标题
cmake_minimum_required(VERSION 3.10.0)
project(SunnyLand VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置编译选项
if (MSVC)
    add_compile_options(/W4)
        add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
        add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置编译输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# 查找并载入Cmake预设
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

# 添加可执行文件
add_executable(${TARGET} "src/engine/core/game_app.h" 
                          "src/engine/core/game_app.cpp" 
                          "src/main.cpp" 
                          "src/engine/core/time.h" 
                          "src/engine/core/time.cpp" 
                          "src/engine/resource/resource_manager.h" 
                          "src/engine/resource/texture_manager.cpp" 
                          "src/engine/resource/audio_manager.cpp" 
                          "src/engine/resource/font_manager.h" 
                          "src/engine/resource/font_manager.cpp" 
                          "src/engine/resource/resource_manager.cpp" 
                          "src/engine/render/renderer.h" 
                          "src/engine/render/renderer.cpp" 
                          "src/engine/render/camera.cpp" 
                          "src/engine/render/camera.h"
                          "src/engine/render/sprite.h"
                          "src/engine/utils/math.h" 
                          "src/engine/core/config.h" 
                          "src/engine/core/config.cpp" 
                          "src/engine/input/input_manager.h" 
                          "src/engine/input/input_manager.cpp"
                          "src/engine/component/component.h"
                          "src/engine/object/game_object.h"
                          "src/engine/object/game_object.cpp")

# 链接库
target_link_libraries(${TARGET}
                        ${SDL3_LIBRARIES}
                        SDL3_image::SDL3_image
                        SDL3_mixer::SDL3_mixer
                        SDL3_ttf::SDL3_ttf
                        glm::glm
                        nlohmann_json::nlohmann_json
                        spdlog::spdlog
                        )