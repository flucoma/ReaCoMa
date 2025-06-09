#include "ReacomaSegmented.h"

namespace iplug {
namespace igraphics {
ReacomaSegmented::ReacomaSegmented(
    const IRECT &bounds, int paramIdx,
    const std::vector<std::string> &segmentLabels, const ReacomaTheme &theme)
    : IControl(bounds, paramIdx), mSegmentLabels(segmentLabels), mTheme(theme) {
    CalculateSegmentRects();
}

void ReacomaSegmented::CalculateSegmentRects() {
    mSegmentRects.clear();
    if (mSegmentLabels.empty())
        return;
    for (size_t i = 0; i < mSegmentLabels.size(); ++i) {
        mSegmentRects.push_back(
            mRECT.SubRectHorizontal(mSegmentLabels.size(), i));
    }
}

void ReacomaSegmented::Draw(IGraphics &g) {
    if (mSegmentLabels.empty())
        return;

    int currentIdx = GetParam() ? GetParam()->Int() : 0;
    const bool isDisabled = IsDisabled();

    // Define colors based on the control's state from the theme
    IColor activeColor = isDisabled ? mTheme.disabledBg : mTheme.active;
    IColor inactiveColor = isDisabled ? mTheme.disabledBg : mTheme.inactive;
    IColor hoverColor = isDisabled
                            ? inactiveColor
                            : mTheme.hover; // Don't show hover when disabled
    IColor textColor = isDisabled ? mTheme.disabled : mTheme.fg;

    IText currentTextStyle = mTheme.buttonStyle;
    currentTextStyle.mFGColor = textColor;

    // 1. Draw each segment's background fill individually
    for (size_t i = 0; i < mSegmentLabels.size(); ++i) {
        IColor segmentFillColor;
        if (i == currentIdx) {
            segmentFillColor = activeColor;
        } else if (i == mHoveredSegment && !isDisabled) {
            segmentFillColor = hoverColor;
        } else {
            segmentFillColor = inactiveColor;
        }
        g.FillRect(segmentFillColor, mSegmentRects[i]);
    }

    // 2. Draw the main border and dividers over everything for a clean look
    g.DrawRoundRect(mTheme.border, mRECT, mTheme.cornerRadius);
    for (size_t i = 0; i < mSegmentLabels.size() - 1; ++i) {
        const IRECT &segmentRect = mSegmentRects[i];
        g.DrawLine(mTheme.border, segmentRect.R, segmentRect.T, segmentRect.R,
                   segmentRect.B);
    }

    // 3. Draw the text labels on top
    for (size_t i = 0; i < mSegmentLabels.size(); ++i) {
        g.DrawText(currentTextStyle, mSegmentLabels[i].c_str(),
                   mSegmentRects[i]);
    }
}

void ReacomaSegmented::OnMouseDown(float x, float y, const IMouseMod &mod) {
    if (IsDisabled())
        return;

    int clickedSegment = GetSegmentForPos(x, y);
    if (clickedSegment != -1 && GetParam()) {
        if (mSegmentLabels.size() > 1) {
            SetValue((double)clickedSegment /
                     (double)(mSegmentLabels.size() - 1));
        } else {
            SetValue(0.0);
        }
        SetDirty(true);
        GetDelegate()->LayoutUI(GetUI());
    }
}

void ReacomaSegmented::OnMouseOver(float x, float y, const IMouseMod &mod) {
    IControl::OnMouseOver(x, y, mod);
    int segment = GetSegmentForPos(x, y);
    if (segment != mHoveredSegment) {
        mHoveredSegment = segment;
        SetDirty(false);
    }
}

void ReacomaSegmented::OnMouseOut() {
    IControl::OnMouseOut();
    if (mHoveredSegment != -1) {
        mHoveredSegment = -1;
        SetDirty(false);
    }
}

int ReacomaSegmented::GetSegmentForPos(float x, float y) {
    if (mSegmentLabels.empty() || !mRECT.Contains(x, y))
        return -1;

    float segmentWidth = mRECT.W() / mSegmentLabels.size();
    int segmentIdx = static_cast<int>((x - mRECT.L) / segmentWidth);
    return std::clamp(segmentIdx, 0, (int)mSegmentLabels.size() - 1);
}
} // namespace igraphics
} // namespace iplug