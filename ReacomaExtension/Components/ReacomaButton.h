#pragma once

#include "IControl.h"
#include "ReacomaTheme.h"

namespace iplug {
namespace igraphics {

class ReacomaButton : public IButtonControlBase {
public:
    ReacomaButton(const IRECT &bounds, const char *label,
                  IActionFunction actionFunction, const ReacomaTheme &theme);

    virtual ~ReacomaButton() = default;

    void Draw(IGraphics &g) override;

    void OnMouseDown(float x, float y, const IMouseMod &mod) override;
    void OnMouseUp(float x, float y, const IMouseMod &mod) override;
    void OnMouseOver(float x, float y, const IMouseMod &mod) override;
    void OnMouseOut() override;

    void SetLabel(const char *label);
    void SetToggleState(bool toggled);

private:
    void UpdateState(bool isOver, bool isPressed);

    ReacomaTheme mTheme;
    WDL_String mLabel;

    IColor mCurrentBackgroundColor;
    bool mIsPressed = false;
    bool mIsToggled = false; // New member to track toggle state
};

} // namespace igraphics
} // namespace iplug