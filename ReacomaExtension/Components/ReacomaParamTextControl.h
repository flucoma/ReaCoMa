#pragma once

#include "IControl.h"
#include "ReacomaTheme.h"

namespace iplug {
namespace igraphics {

class ReacomaParamTextControl : public IControl {
public:
    virtual ~ReacomaParamTextControl() = default;
    ReacomaParamTextControl(const IRECT &bounds, int paramIdx,
                            const ReacomaTheme &theme);

    void Draw(IGraphics &g) override;
    void OnMouseDown(float x, float y, const IMouseMod &mod) override;
    void OnMouseOver(float x, float y, const IMouseMod &mod) override;
    void OnMouseOut() override;

private:
    ReacomaTheme mTheme;
    IRECT mLabelBounds;
    IRECT mValueBounds;
    bool mMouseOverValueBox = false;
};

} // namespace igraphics
} // namespace iplug