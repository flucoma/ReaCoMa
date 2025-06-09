#pragma once

#include "IControl.h"
#include <string>
#include <vector>
#include "ReacomaTheme.h"

namespace iplug {
namespace igraphics {
class ReacomaSegmented : public IControl {
public:
    ReacomaSegmented(const IRECT &bounds, int paramIdx,
                     const std::vector<std::string> &segmentLabels,
                     const ReacomaTheme &theme);
    virtual ~ReacomaSegmented() = default;

    void Draw(IGraphics &g) override;
    void OnMouseDown(float x, float y, const IMouseMod &mod) override;
    void OnMouseOver(float x, float y, const IMouseMod &mod) override;
    void OnMouseOut() override;

private:
    std::vector<std::string> mSegmentLabels;
    std::vector<IRECT> mSegmentRects;
    ReacomaTheme mTheme;

    int mHoveredSegment = -1;

    void CalculateSegmentRects();
    int GetSegmentForPos(float x, float y);
};
} // namespace igraphics
} // namespace iplug
