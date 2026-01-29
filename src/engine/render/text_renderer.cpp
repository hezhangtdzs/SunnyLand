#include "text_renderer.h"
#include "camera.h"
#include <stdexcept>
#include <string>
#include <glm/glm.hpp>
#include "../utils/math.h"

#include "../resource/resource_manager.h"
#include "../resource/font_manager.h"

namespace engine::render {

    // 静态变量，用于跟踪TTF库的初始化状态
    static bool ttf_initialized = false;

    /**
     * @brief 构造 TextRenderer 实例。
     * @param sdl_renderer SDL 渲染器指针
     * @param resource_manager 资源管理器指针
     * @throw std::runtime_error 如果初始化失败
     */
    TextRenderer::TextRenderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager)
        : sdl_renderer_(sdl_renderer), resource_manager_(resource_manager) {
        
        // 初始化 SDL3_ttf 库
        if (!ttf_initialized) {
            if (TTF_Init() < 0) {
                throw std::runtime_error("Failed to initialize SDL3_ttf: " + std::string(SDL_GetError()));
            }
            ttf_initialized = true;
        }
        
        // 初始化 TTF_TextEngine
        text_engine_ = TTF_CreateRendererTextEngine(sdl_renderer_);
        if (!text_engine_) {
            throw std::runtime_error("Failed to create TTF_TextEngine: " + std::string(SDL_GetError()));
        }
    }
    
    /**
     * @brief 析构函数，释放资源。
     */
    TextRenderer::~TextRenderer() {
        if (text_engine_) {
            TTF_DestroyRendererTextEngine(text_engine_);
            text_engine_ = nullptr;
        }
        
        // 关闭 SDL3_ttf 库（仅当所有TextRenderer实例都被销毁时）
        if (ttf_initialized) {
            TTF_Quit();
            ttf_initialized = false;
        }
    }
    
    /**
     * @brief 在世界空间中绘制文本（跟随相机移动）。
     * @param camera 用于计算屏幕坐标的相机
     * @param text 要绘制的文本字符串
     * @param font_path 字体文件路径
     * @param font_size 字体大小（点值）
     * @param position 文本的世界空间位置
     * @param color 文本颜色
     */
    void TextRenderer::drawText(const Camera& camera,
                               const std::string& text,
                               const std::string& font_path,
                               int font_size,
                               const glm::vec2& position,
                               const engine::utils::FColor& color) {
        // 获取字体
        TTF_Font* font = resource_manager_->getFont(font_path, font_size);
        if (!font) {
            return;
        }
        
        // 将世界坐标转换为屏幕坐标
        glm::vec2 screen_pos = camera.worldToScreen(position);
        
        // 创建文本对象
        TTF_Text* ttf_text = TTF_CreateText(text_engine_, font, text.c_str(), 0);
        if (!ttf_text) {
            return;
        }
        TTF_SetTextColorFloat(ttf_text, 0.0f, 0.0f, 0.0f, 1.0f);
        TTF_DrawRendererText(ttf_text, position.x + 2, position.y + 2);
        
        TTF_SetTextColorFloat(ttf_text, color.r, color.g, color.b, color.a);
        
        // 绘制文本
        TTF_DrawRendererText(ttf_text, screen_pos.x, screen_pos.y);
        
        // 销毁文本对象
        TTF_DestroyText(ttf_text);
    }
    
    /**
     * @brief 在屏幕空间中绘制文本（UI 层，不跟随相机）。
     * @param text 要绘制的文本字符串
     * @param font_path 字体文件路径
     * @param font_size 字体大小（点值）
     * @param position 文本的屏幕坐标位置
     * @param color 文本颜色
     */
    void TextRenderer::drawUIText(const std::string& text,
                                 const std::string& font_path,
                                 int font_size,
                                 const glm::vec2& position,
                                 const engine::utils::FColor& color) {
        // 获取字体
        TTF_Font* font = resource_manager_->getFont(font_path, font_size);
        if (!font) {
            return;
        }
        
        // 创建文本对象
        TTF_Text* ttf_text = TTF_CreateText(text_engine_, font, text.c_str(), 0);
        if (!ttf_text) {
            return;
        }
        
        TTF_SetTextColorFloat(ttf_text, 0.0f, 0.0f, 0.0f, 1.0f);
        TTF_DrawRendererText(ttf_text, position.x + 2, position.y + 2); 

        TTF_SetTextColorFloat(ttf_text, color.r, color.g, color.b, color.a);
        
        // 直接使用屏幕坐标绘制文本
        TTF_DrawRendererText(ttf_text, position.x, position.y);
        
        // 销毁文本对象
        TTF_DestroyText(ttf_text);
    }

    /**
     * @brief 获取文本的渲染尺寸。
     * @param text 文本内容
     * @param font_path 字体文件路径
     * @param font_size 字体大小（点值）
     * @return 文本的宽度和高度
     */
    glm::vec2 TextRenderer::getTextSize(const std::string& text, const std::string& font_path, int font_size) {
        TTF_Font* font = resource_manager_->getFont(font_path, font_size);
        if (!font) {
            return { 0.0f, 0.0f };
        }

        TTF_Text* ttf_text = TTF_CreateText(text_engine_, font, text.c_str(), 0);
        if (!ttf_text) {
            return { 0.0f, 0.0f };
        }

        int w, h;
        TTF_GetTextSize(ttf_text, &w, &h);
        TTF_DestroyText(ttf_text);

        return { static_cast<float>(w), static_cast<float>(h) };
    }
}