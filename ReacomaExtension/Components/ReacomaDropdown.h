#pragma once

#include "IControl.h"
#include <functional>
#include <memory>
#include "IGraphicsPopupMenu.h"
#include "ReacomaExtension.h"

class ReacomaDropdown : public IControl {
  public:
    using ActionFunction = std::function<void(int selectedIdx)>;

    ReacomaDropdown(const IRECT &bounds, int paramIdx,
                    const IVStyle &style = DEFAULT_STYLE)
        : IControl(bounds, paramIdx), mStyle(style) {
        mStyle.valueText.mVAlign = EVAlign::Middle;
    }

    void Draw(IGraphics &g) override;
    void OnMouseDown(float x, float y, const IMouseMod &mod) override;

    void SetActionFunction(ActionFunction func) { mActionFunc = func; }

  private:
    IVStyle mStyle;
    ActionFunction mActionFunc = nullptr;
    std::unique_ptr<IPopupMenu> mMenu;
};