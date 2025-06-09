#include "ReacomaProgressBar.h"
#include <iomanip>
#include <sstream>

namespace iplug {
namespace igraphics {
ReacomaProgressBar::ReacomaProgressBar(const IRECT &bounds, const char *label,
                                       const ReacomaTheme &theme)
    : IControl(bounds), mTheme(theme), mProgress(0.0), mLabel(label) {}

void ReacomaProgressBar::Draw(IGraphics &g) {
    // Use theme colors for the bar
    g.FillRect(mTheme.inactive, mRECT); // Track color

    IRECT progressRect = mRECT.GetFromLeft(mRECT.W() * mProgress);
    g.FillRect(mTheme.hover, progressRect); // Fill color

    g.DrawRect(mTheme.border, mRECT); // Frame color

    // Use theme text style for the percentage
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << (mProgress * 100.0) << "%";
    std::string progressStr = ss.str();

    IText textStyle = mTheme.valueStyle;
    textStyle.mFGColor = mTheme.fg; // Ensure text color is from the theme

    g.DrawText(textStyle, progressStr.c_str(), mRECT);

    if (IsDisabled()) {
        g.FillRect(mTheme.bg.WithOpacity(0.7f), mRECT, &BLEND_75);
    }
}
} // namespace igraphics
} // namespace iplug
