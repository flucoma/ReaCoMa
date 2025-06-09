#pragma once

#include "IControl.h"
#include "IGraphicsStructs.h"

using namespace iplug::igraphics;

struct ReacomaTheme {
    // Colors
    IColor bg = COLOR_WHITE;
    IColor fg = COLOR_BLACK;
    IColor pressed = IColor(255, 100, 180, 100);
    IColor active = COLOR_LIGHT_GRAY.WithContrast(-0.1f);
    IColor inactive = COLOR_WHITE;
    IColor hover = COLOR_LIGHT_GRAY;
    IColor border = COLOR_BLACK;
    IColor disabled =
        IColor(255, 170, 170, 170); // Lighter gray for text/borders
    IColor disabledBg =
        IColor(255, 245, 245, 245); // Very light gray for backgrounds

    // Text Styles
    IText labelStyle =
        IText(14.f, fg, "ibmplex", EAlign::Near, EVAlign::Middle);
    IText valueStyle =
        IText(14.f, fg, "ibmplex", EAlign::Center, EVAlign::Middle);
    IText buttonStyle =
        IText(14.f, fg, "ibmplex", EAlign::Center, EVAlign::Middle);

    // Dimensions
    float cornerRadius = 0.f;
    float borderThickness = 1.f;
    float padding = 2.f;

    static ReacomaTheme CreateDefault() { return ReacomaTheme{}; }
};