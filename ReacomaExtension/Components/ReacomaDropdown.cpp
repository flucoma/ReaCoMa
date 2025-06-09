#include "ReacomaDropdown.h"
#include "ReacomaExtension.h"

void ReacomaDropdown::Draw(IGraphics &g) {
    g.FillRoundRect(mStyle.colorSpec.GetColor(kBG), mRECT, mStyle.roundness);

    if (GetParam()) {
        WDL_String valStr;
        GetParam()->GetDisplay(valStr);
        g.DrawText(mStyle.valueText, valStr.Get(), mRECT);
    }

    const float arrowSize = 8.f;
    IRECT arrowRect = mRECT.GetFromRight(20.f).GetCentredInside(arrowSize);
    g.FillTriangle(mStyle.colorSpec.GetColor(kFG), arrowRect.L, arrowRect.T,
                   arrowRect.R, arrowRect.T, arrowRect.MW(), arrowRect.B);
    g.DrawRoundRect(mStyle.colorSpec.GetColor(kFG), mRECT, mStyle.roundness,
                    nullptr, mStyle.frameThickness);
}

void ReacomaDropdown::OnMouseDown(float x, float y, const IMouseMod &mod) {
    IGraphics *pGraphics = GetUI();
    IParam *pAlgoParam = GetDelegate()->GetParam(GetParamIdx());

    if (!pGraphics || !GetParam())
        return;

    std::vector<const char *> items;
    for (int i = 0; i < GetParam()->GetMax() + 1; ++i) {
        items.push_back(GetParam()->GetDisplayTextAtIdx(i));
    }

    IPopupMenu menu("Algorithms");

    // Set the action function that will execute when an item is chosen.
    menu.SetFunction([this](IPopupMenu *pMenu) {
        if (pMenu->GetChosenItemIdx() > -1) {
            int selectedIdx = pMenu->GetChosenItemIdx();

            // Get a fresh pointer to the parameter inside the lambda to ensure
            // it's valid.
            IParam *pAlgoParamInsideLambda =
                GetDelegate()->GetParam(GetParamIdx());
            if (pAlgoParamInsideLambda)
                pAlgoParamInsideLambda->Set(selectedIdx);

            // If an action function has been set from the outside, call it.
            if (mActionFunc) {
                mActionFunc(selectedIdx);
            }
        }
    });

    // Loop through the parameter's display texts and add each one as an item to
    // the menu.
    for (int i = 0; i < pAlgoParam->GetMax() + 1; ++i) {
        menu.AddItem(pAlgoParam->GetDisplayTextAtIdx(i));
    }

    // Show the prepared menu.
    pGraphics->CreatePopupMenu(*this, menu, mRECT);

    SetDirty(false);
}
