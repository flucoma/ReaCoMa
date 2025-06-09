#include "ReacomaParamTextControl.h"
#include <algorithm>

namespace iplug {
namespace igraphics {

ReacomaParamTextControl::ReacomaParamTextControl(const IRECT &bounds,
                                                 int paramIdx,
                                                 const ReacomaTheme &theme)
    : IControl(bounds, paramIdx), mTheme(theme) {
    const float valueBoxWidth = 65.f;
    const float interLabelPadding = 10.f;

    float clampedWidth = std::min(valueBoxWidth, bounds.W());
    mValueBounds = mRECT.GetFromLeft(clampedWidth);
    mLabelBounds =
        mRECT.GetFromRight(mRECT.W() - clampedWidth - interLabelPadding);
}

void ReacomaParamTextControl::Draw(IGraphics &g) {
    const IParam *pParam = GetParam();
    const bool isDisabled = IsDisabled();

    char paramName[128] = "ERROR";
    WDL_String paramDisplay;

    if (pParam) {
        strcpy(paramName, pParam->GetName());
        pParam->GetDisplay(paramDisplay);
    }

    // Determine colors from theme based on state
    IColor textColor = isDisabled ? mTheme.disabled : mTheme.fg;
    IColor valueBoxBG = isDisabled
                            ? mTheme.disabledBg
                            : (mMouseOverValueBox ? mTheme.hover : mTheme.bg);

    // Prepare text styles from theme
    IText labelStyle = mTheme.labelStyle;
    labelStyle.mFGColor = textColor;

    IText valueStyle = mTheme.valueStyle;
    valueStyle.mFGColor = textColor;

    g.FillRect(valueBoxBG, mValueBounds);
    g.DrawRect(mTheme.border, mValueBounds);
    g.DrawText(labelStyle, paramName, mLabelBounds.GetPadded(-5.f));
    g.DrawText(valueStyle, paramDisplay.Get(), mValueBounds.GetPadded(-5.f));
}

void ReacomaParamTextControl::OnMouseDown(float x, float y,
                                          const IMouseMod &mod) {
    if (mValueBounds.Contains(x, y) && !IsDisabled()) {
        WDL_String currentText;
        if (GetParam())
            GetParam()->GetDisplay(currentText);

        GetUI()->CreateTextEntry(*this, mTheme.valueStyle, mValueBounds,
                                 currentText.Get());
    }
}

void ReacomaParamTextControl::OnMouseOver(float x, float y,
                                          const IMouseMod &mod) {
    bool isOverValue = mValueBounds.Contains(x, y);
    if (isOverValue != mMouseOverValueBox) {
        mMouseOverValueBox = isOverValue;
        SetDirty(false);
    }
    IControl::OnMouseOver(x, y, mod);
}

void ReacomaParamTextControl::OnMouseOut() {
    if (mMouseOverValueBox) {
        mMouseOverValueBox = false;
        SetDirty(false);
    }
    IControl::OnMouseOut();
}

} // namespace igraphics
} // namespace iplug