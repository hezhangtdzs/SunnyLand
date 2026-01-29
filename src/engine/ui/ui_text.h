#pragma once
/**
 * @file ui_text.h
 * @brief 定义UIText类，用于显示文本内容。
 */

#include "ui_element.h"
#include <string>
#include <glm/glm.hpp>
#include "../utils/math.h" // 用于FColor定义

using namespace engine::utils;

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @enum TextAlignment
 * @brief 文本对齐方式枚举。
 */
enum class TextAlignment {
    LEFT,   ///< 左对齐
    CENTER, ///< 居中对齐
    RIGHT   ///< 右对齐
};

/**
 * @class UIText
 * @brief UI文本类，用于显示文本内容。
 * 
 * UIText类主要功能包括：
 * - 显示文本内容
 * - 支持设置字体和字体大小
 * - 支持设置文本颜色
 * - 支持设置文本对齐方式
 */
class UIText : public UIElement {
private:
    /// 文本内容
    std::string text_;
    /// 字体路径
    std::string font_path_;
    /// 字体大小
    int font_size_ = 16;
    /// 文本颜色
    FColor color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
    /// 文本对齐方式
    TextAlignment alignment_ = TextAlignment::LEFT;

public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     * @param text 初始文本内容。
     * @param font_path 字体文件路径。
     * @param font_size 字体大小。
     */
    UIText(engine::core::Context& context, const std::string& text, const std::string& font_path, int font_size = 16);

    /**
     * @brief 析构函数。
     */
    ~UIText() override;

    /**
     * @brief 渲染文本。
     */
    void render() override;

    // Getters and Setters
    /**
     * @brief 获取文本内容。
     * @return 文本内容。
     */
    const std::string& getText() const { return text_; }

    /**
     * @brief 设置文本内容。
     * @param text 文本内容。
     */
    void setText(const std::string& text) { 
        text_ = text; 
        updateSize();
    }

    /**
     * @brief 获取字体路径。
     * @return 字体路径。
     */
    const std::string& getFontPath() const { return font_path_; }

    /**
     * @brief 设置字体路径。
     * @param font_path 字体路径。
     */
    void setFontPath(const std::string& font_path) { 
        font_path_ = font_path; 
        updateSize();
    }

    /**
     * @brief 获取字体大小。
     * @return 字体大小。
     */
    int getFontSize() const { return font_size_; }

    /**
     * @brief 设置字体大小。
     * @param font_size 字体大小。
     */
    void setFontSize(int font_size) { 
        font_size_ = font_size; 
        updateSize();
    }

    /**
     * @brief 获取文本颜色。
     * @return 文本颜色。
     */
    const FColor& getColor() const { return color_; }

    /**
     * @brief 设置文本颜色。
     * @param color 文本颜色。
     */
    void setColor(const FColor& color) { color_ = color; }

    /**
     * @brief 获取文本对齐方式。
     * @return 文本对齐方式。
     */
    TextAlignment getAlignment() const { return alignment_; }

    /**
     * @brief 设置文本对齐方式。
     * @param alignment 文本对齐方式。
     */
    void setAlignment(TextAlignment alignment) { alignment_ = alignment; }

private:
    /**
     * @brief 更新文本的尺寸。
     */
    void updateSize();
};

}