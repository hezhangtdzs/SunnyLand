#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>


int main(int, char**) {

    spdlog::info("你好，世界!");
    nlohmann::json json_data = { {"a",10} };
    auto num = json_data["a"].get<int>();
    spdlog::warn("json: {}", num);

    std::cout << "Hello, World!" << std::endl;
    // SDL初始化
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    // 创建窗口
    SDL_Window* window = SDL_CreateWindow("Hello World!", 800, 600, 0);
    // 创建渲染器
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // SDL3_Image不需要手动初始化

    // 加载图片
    SDL_Texture* texture = IMG_LoadTexture(renderer, "assets/textures/UI/title-screen.png");
    if (!texture) {
        std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << std::endl;
    }

    // SDL_Mixer初始化
    if (!MIX_Init()) {
        printf("Mixer init failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 44100 };
    // 获取默认音频设备ID
    SDL_AudioDeviceID device_id = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
    MIX_Mixer* mixer = MIX_CreateMixerDevice(device_id, &spec);
    MIX_Audio* audio = MIX_LoadAudio(mixer, "assets/audio/platformer_level03_loop.ogg", false);
    MIX_Track* track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, audio);
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);  // -1=无限循环
    MIX_PlayTrack(track, props);
    SDL_DestroyProperties(props);

    // SDL_TTF初始化
    if (!TTF_Init()) {
        std::cerr << "TTF_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    // 加载字体
    const char* fontPath = "assets/fonts/VonwaonBitmap-16px.ttf";
    TTF_Font* font = TTF_OpenFont(fontPath, 24);
    if (!font) {
        std::cerr << "TTF_OpenFont Error (" << fontPath << "): " << SDL_GetError() << std::endl;
        return 1;
    }

    // 创建文本纹理
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Hello, SDL! 中文也可以", 0, color);
    SDL_Texture* textTexture = nullptr;
    float textW = 0.0f;
    float textH = 0.0f;

    if (surface) {
        textTexture = SDL_CreateTextureFromSurface(renderer, surface);
        // 缓存宽高，避免在循环中访问 surface
        textW = static_cast<float>(surface->w);
        textH = static_cast<float>(surface->h);
        // 创建纹理后，surface 不再需要，应立即释放
        SDL_DestroySurface(surface);
        surface = nullptr;
    }
    else {
        std::cerr << "TTF_RenderText_Solid Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Do something with the window and renderer here...
    // 渲染循环
    while (true) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                break;
            }
        }

        // 清屏
        SDL_RenderClear(renderer);
        // 画一个长方形
        SDL_FRect rect = { 100, 100, 200, 200 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);


        // 画图片
        if (texture) {
            SDL_FRect dstrect = { 200, 200, 200, 200 };
            SDL_RenderTexture(renderer, texture, NULL, &dstrect);
        }

        // 画文本
        if (textTexture) {
            // 使用缓存的宽高，不再解引用已被释放的 surface
            SDL_FRect textRect = { 300, 300, textW, textH };
            SDL_RenderTexture(renderer, textTexture, NULL, &textRect);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // 更新屏幕
        SDL_RenderPresent(renderer);



    }

    // 清理图片资源
    if (texture) SDL_DestroyTexture(texture);

    // 清理音乐资源
    MIX_DestroyTrack(track);
    MIX_DestroyAudio(audio);
    MIX_DestroyMixer(mixer);  // 关闭设备
    MIX_Quit();

    // 清理字体资源
    // surface 已在前面释放，这里不再需要 SDL_DestroySurface(surface)
    if (textTexture) SDL_DestroyTexture(textTexture);
    if (font) TTF_CloseFont(font);
    TTF_Quit();

    // 清理并退出
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}