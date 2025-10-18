#pragma once

#include "ReaperExt_include_in_plug_hdr.h"
#include "reaper_plugin.h"
#include "IControl.h"

#include "ibmplexmono.hpp"
#include "roboto.hpp"

#include <chrono>
#include <list>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "IAlgorithm.h"

namespace iplug {
namespace igraphics {
class ReacomaButton;
class ReacomaProgressBar;
class ReacomaSegmented;
class ReacomaParamTextControl;
} // namespace igraphics
} // namespace iplug

class HPSSAlgorithm;
class NMFAlgorithm;
class TransientAlgorithm;
class TransientSliceAlgorithm;
class NoveltySliceAlgorithm;
class OnsetSliceAlgorithm;
class AmpGateAlgorithm;
class NoveltyFeatureAlgorithm;
class AmpSliceAlgorithm;
struct ReacomaTheme;

class IAlgorithm;
class ProcessingJob;

using namespace iplug;
using namespace igraphics;

class ReacomaExtension : public ReaperExtBase {

public:
    enum class Mode { Segment, Regions, ProcessAudio };
    Mode GetCurrentMode() const { return mCurrentProcessingMode; }

    enum EParams { kParamAlgorithmChoice = 0, kNumOwnParams };

    enum EControlTags { kCtrlTagAlgoChooser = 0 };

    enum EAlgorithmChoice {
        kNoveltySlice = 0,
        kAmpSlice,
        kAmpGate,
        kOnsetSlice,
        kTransientSlice,
        kHPSS,
        kNMF,
        kTransients,
        kNumAlgorithmChoices
    };

    ReacomaExtension(reaper_plugin_info_t *pRec);
    void OnUIClose() override;
    void Process(Mode mode, bool force);
    void CancelRunningJobs();
    void ResetUIState();
    void SetAlgorithmChoice(EAlgorithmChoice choice, bool triggerUIRelayout);
    void UpdateAutoProcessButtonState();

    NoveltySliceAlgorithm *GetNoveltySliceAlgorithm() const {
        return mNoveltyAlgorithm.get();
    }
    HPSSAlgorithm *GetHPSSAlgorithm() const { return mHPSSAlgorithm.get(); }
    NMFAlgorithm *GetNMFAlgorithm() const { return mNMFAlgorithm.get(); }
    TransientAlgorithm *GetTransientsAlgorithm() const {
        return mTransientsAlgorithm.get();
    }
    TransientSliceAlgorithm *GetTransientSliceAlgorithm() const {
        return mTransientSliceAlgorithm.get();
    }
    OnsetSliceAlgorithm *GetOnsetSliceAlgorithm() const {
        return mOnsetSliceAlgorithm.get();
    }
    AmpGateAlgorithm *GetAmpGateAlgorithm() const {
        return mAmpGateAlgorithm.get();
    }
    NoveltyFeatureAlgorithm *GetNoveltyFeatureAlgorithm() const {
        return mNoveltyFeatureAlgorithm.get();
    }
    AmpSliceAlgorithm *GetAmpSliceAlgorithm() const {
        return mAmpSliceAlgorithm.get();
    }

private:
    bool mUIRelayoutIsNeeded = false;

    std::unique_ptr<ReacomaTheme> mTheme;

    std::unique_ptr<NoveltySliceAlgorithm> mNoveltyAlgorithm;
    std::unique_ptr<HPSSAlgorithm> mHPSSAlgorithm;
    std::unique_ptr<NMFAlgorithm> mNMFAlgorithm;
    std::unique_ptr<TransientAlgorithm> mTransientsAlgorithm;
    std::unique_ptr<OnsetSliceAlgorithm> mOnsetSliceAlgorithm;
    std::unique_ptr<TransientSliceAlgorithm> mTransientSliceAlgorithm;
    std::unique_ptr<AmpGateAlgorithm> mAmpGateAlgorithm;
    std::unique_ptr<NoveltyFeatureAlgorithm> mNoveltyFeatureAlgorithm;
    std::unique_ptr<AmpSliceAlgorithm> mAmpSliceAlgorithm;
    std::vector<IAlgorithm *> mAllAlgorithms;

    void OnParamChangeUI(int paramIdx, EParamSource source) override;
    void OnIdle() override;
    void SetupUI(IGraphics *pGraphics);
    void StartNextItemInQueue();
    void SaveState();
    void LoadState();
    std::string GetSettingsFilePath() const;

    int mGUIToggle = 0;

    IAlgorithm *mCurrentActiveAlgorithmPtr = nullptr;
    EAlgorithmChoice mCurrentAlgorithmChoice = kNoveltySlice;
    Mode mCurrentProcessingMode;

    unsigned int mConcurrencyLimit = 1;
    std::deque<MediaItem *> mPendingItemsQueue;
    std::list<std::unique_ptr<ProcessingJob>> mActiveJobs;
    std::deque<std::unique_ptr<ProcessingJob>> mFinalizationQueue;
    std::deque<MediaItem *> mProcessingQueue;

    iplug::igraphics::ReacomaProgressBar *mProgressBar = nullptr;
    iplug::igraphics::ReacomaButton *mCancelButton = nullptr;
    iplug::igraphics::ReacomaButton *mAutoProcessButton = nullptr;
    ITextControl *mProcessingLabel = nullptr;
    int mProcessingLabelIdx = -1;
    size_t mTotalBatchItems = 0;
    double mLastReportedProgress = 0.0;

    ReaProject *mBatchUndoProject = nullptr;
    bool mIsProcessingBatch = false;
    bool mIsCancellationRequested = false;

    // handle different processing modes
    bool mAutoProcessMode = false;
    bool mProcessIsPending = false;
    bool mHasUserInteractedSinceLoad = false;
    bool mStateLoaded = false;
    std::chrono::steady_clock::time_point mLastParamChangeTime;
    static constexpr auto AUTO_PROCESS_DELAY = std::chrono::milliseconds(50);

    // Ellipsis animation
    int mEllipsisCount = 0;
    std::chrono::steady_clock::time_point mLastEllipsisUpdateTime;
    static constexpr auto ELLIPSIS_UPDATE_DELAY =
        std::chrono::milliseconds(300);
};