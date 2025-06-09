#pragma once

#include "IControl.h"
#include "ReacomaTheme.h"
#include <algorithm>

namespace iplug {
namespace igraphics {
class ReacomaProgressBar : public IControl {
public:
    virtual ~ReacomaProgressBar() = default;
    ReacomaProgressBar(const IRECT &bounds, const char *label,
                       const ReacomaTheme &theme);

    void Draw(IGraphics &g) override;

    void SetProgress(double progress) {
        mProgress = std::clamp(progress, 0.0, 1.0);
        SetDirty(false);
    }

private:
    ReacomaTheme mTheme;
    double mProgress;
    WDL_String mLabel;
};
} // namespace igraphics
} // namespace iplug
