#include "ReacomaButton.h"

namespace iplug {
namespace igraphics {

ReacomaButton::ReacomaButton(const IRECT &bounds, const char *label,
                             IActionFunction actionFunction,
                             const ReacomaTheme &theme)
    : IButtonControlBase(bounds, actionFunction), mTheme(theme), mLabel(label),
      mCurrentBackgroundColor(theme.inactive), mIsPressed(false) {
    SetTooltip(label);
}

void ReacomaButton::Draw(IGraphics &g) {
    const bool isDisabled = IsDisabled();

    IColor bgColor = isDisabled ? mTheme.disabledBg : mCurrentBackgroundColor;
    IColor fgColor = isDisabled ? mTheme.disabled : mTheme.fg;
    IColor borderColor = isDisabled ? mTheme.disabled : mTheme.border;

    IRECT areaToDraw = mRECT;
    if (mTheme.borderThickness > 0.f) {
        areaToDraw.Pad(mTheme.borderThickness / 2.f);
    }

    if (mTheme.cornerRadius > 0.f) {
        g.FillRoundRect(bgColor, areaToDraw, mTheme.cornerRadius, &mBlend);
        if (mTheme.borderThickness > 0.f)
            g.DrawRoundRect(borderColor, areaToDraw, mTheme.cornerRadius,
                            &mBlend, mTheme.borderThickness);
    } else {
        g.FillRect(bgColor, areaToDraw, &mBlend);
        if (mTheme.borderThickness > 0.f)
            g.DrawRect(borderColor, areaToDraw, &mBlend,
                       mTheme.borderThickness);
    }

    if (mLabel.GetLength()) {
        IText currentTextStyle = mTheme.buttonStyle;
        currentTextStyle.mFGColor =
            fgColor; // Use the state-dependent foreground color
        g.DrawText(currentTextStyle, mLabel.Get(), mRECT, &mBlend);
    }
}

void ReacomaButton::UpdateState(bool isOver, bool isPressed) {
    IColor targetColor;

    if (isPressed) {
        targetColor = mTheme.inactive;
    } else if (mIsToggled) {
        targetColor = mTheme.inactive;
    } else if (isOver) {
        targetColor = mTheme.hover;
    } else {
        targetColor = mTheme.inactive;
    }

    if (mCurrentBackgroundColor != targetColor) {
        mCurrentBackgroundColor = targetColor;
        SetDirty(false);
    }
}

void ReacomaButton::SetToggleState(bool toggled) {
    if (mIsToggled != toggled) {
        mIsToggled = toggled;
        UpdateState(mMouseIsOver, mIsPressed);
    }
}

void ReacomaButton::OnMouseOver(float x, float y, const IMouseMod &mod) {
    IControl::OnMouseOver(x, y, mod);
    if (!mIsPressed)
        UpdateState(true, false);
}

void ReacomaButton::OnMouseOut() {
    IControl::OnMouseOut();
    if (!mIsPressed)
        UpdateState(false, false);
}

void ReacomaButton::OnMouseDown(float x, float y, const IMouseMod &mod) {
    mIsPressed = true;
    UpdateState(true, true);
    IButtonControlBase::OnMouseDown(x, y, mod);
}

void ReacomaButton::OnMouseUp(float x, float y, const IMouseMod &mod) {
    mIsPressed = false;
    UpdateState(mMouseIsOver, false);
    IButtonControlBase::OnMouseUp(x, y, mod);
}

void ReacomaButton::SetLabel(const char *label) {
    mLabel.Set(label);
    SetTooltip(label);
    SetDirty(false);
}

} // namespace igraphics
} // namespace iplug