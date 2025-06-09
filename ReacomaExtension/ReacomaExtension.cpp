#include "IControl.h"
#include "IControls.h"
#include "IGraphicsStructs.h"
#define REAPERAPI_IMPLEMENT

#include "ReacomaExtension.h"
#include "ReaperExt_include_in_plug_src.h"

#include <fstream>
#include <chrono>

#include "ReacomaTheme.h"
#include "Algorithms/ProcessingJob.h"
#include "Components/ReacomaButton.h"
#include "Components/ReacomaParamTextControl.h"
#include "Components/ReacomaProgressBar.h"
#include "Components/ReacomaSegmented.h"
#include "Algorithms/HPSSAlgorithm.h"
#include "Algorithms/NMFAlgorithm.h"
#include "Algorithms/TransientAlgorithm.h"
#include "Algorithms/TransientSliceAlgorithm.h"
#include "Algorithms/NoveltySliceAlgorithm.h"
#include "Algorithms/OnsetSliceAlgorithm.h"
#include "Algorithms/AmpGateAlgorithm.h"
#include "Algorithms/NoveltyFeatureAlgorithm.h"

template <ReacomaExtension::Mode M> struct ProcessAction {
    void operator()(IControl *pCaller) {
        static_cast<ReacomaExtension *>(pCaller->GetDelegate())
            ->Process(M, true);
    }
};

ReacomaExtension::ReacomaExtension(reaper_plugin_info_t *pRec)
    : ReaperExtBase(pRec) {

    mTheme = std::make_unique<ReacomaTheme>();

    IMPAPI(CountSelectedMediaItems);
    IMPAPI(GetSelectedMediaItem);
    IMPAPI(GetItemProjectContext);
    IMPAPI(GetActiveTake);
    IMPAPI(GetMediaItemTake_Source);
    IMPAPI(GetMediaSourceSampleRate);
    IMPAPI(GetMediaSourceNumChannels);
    IMPAPI(GetMediaItemTakeInfo_Value);
    IMPAPI(GetMediaItemInfo_Value);
    IMPAPI(GetMediaSourceLength);
    IMPAPI(GetNumTakeMarkers);
    IMPAPI(IsMediaItemSelected);
    IMPAPI(DeleteTakeMarker);
    IMPAPI(ColorToNative);
    IMPAPI(SplitMediaItem);
    IMPAPI(DeleteTrackMediaItem);
    IMPAPI(SetTakeMarker);
    IMPAPI(AddProjectMarker2);
    IMPAPI(GetMediaItem_Track);
    IMPAPI(Undo_BeginBlock2);
    IMPAPI(Undo_EndBlock2);
    IMPAPI(UpdateArrange);
    IMPAPI(UpdateTimeline);
    IMPAPI(PCM_Source_CreateFromSimple);
    IMPAPI(AddTakeToMediaItem);
    IMPAPI(GetSetMediaItemTakeInfo);
    IMPAPI(PCM_Source_BuildPeaks);
    IMPAPI(GetProjectPath);
    IMPAPI(PCM_Sink_Create);
    IMPAPI(PCM_Sink_CreateEx);
    IMPAPI(PCM_Source_CreateFromFile);
    IMPAPI(GetMediaSourceParent);
    IMPAPI(GetMediaSourceFileName);
    IMPAPI(GetProjectPathEx);
    IMPAPI(GetSetProjectInfo_String);
    IMPAPI(SetMediaItemInfo_Value);

    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS);
    };

    RegisterAction(
        "Reacoma: Show/Hide UI",
        [&]() {
            ShowHideMainWindow();
            mGUIToggle = !mGUIToggle;
        },
        true, &mGUIToggle);

    AddParam();
    auto parameterLabels = {
        "Novelty Slice", "Onset Slice", "Transient Slice", "HPSS",
        "NMF",           "Transients",  "Amp Gate",        "Novelty Feature"};
    GetParam(kParamAlgorithmChoice)
        ->InitEnum("Algorithm", kNoveltySlice, parameterLabels);

    mNoveltyAlgorithm = std::make_unique<NoveltySliceAlgorithm>(this);
    mNoveltyAlgorithm->RegisterParameters();

    mHPSSAlgorithm = std::make_unique<HPSSAlgorithm>(this);
    mHPSSAlgorithm->RegisterParameters();

    mNMFAlgorithm = std::make_unique<NMFAlgorithm>(this);
    mNMFAlgorithm->RegisterParameters();

    mOnsetSliceAlgorithm = std::make_unique<OnsetSliceAlgorithm>(this);
    mOnsetSliceAlgorithm->RegisterParameters();

    mTransientsAlgorithm = std::make_unique<TransientAlgorithm>(this);
    mTransientsAlgorithm->RegisterParameters();

    mTransientSliceAlgorithm = std::make_unique<TransientSliceAlgorithm>(this);
    mTransientSliceAlgorithm->RegisterParameters();

    mAmpGateAlgorithm = std::make_unique<AmpGateAlgorithm>(this);
    mAmpGateAlgorithm->RegisterParameters();

    mNoveltyFeatureAlgorithm = std::make_unique<NoveltyFeatureAlgorithm>(this);
    mNoveltyFeatureAlgorithm->RegisterParameters();

    mAllAlgorithms.push_back(mNoveltyAlgorithm.get());
    mAllAlgorithms.push_back(mHPSSAlgorithm.get());
    mAllAlgorithms.push_back(mNMFAlgorithm.get());
    mAllAlgorithms.push_back(mOnsetSliceAlgorithm.get());
    mAllAlgorithms.push_back(mTransientsAlgorithm.get());
    mAllAlgorithms.push_back(mTransientSliceAlgorithm.get());
    mAllAlgorithms.push_back(mAmpGateAlgorithm.get());
    mAllAlgorithms.push_back(mNoveltyFeatureAlgorithm.get());

    SetAlgorithmChoice(kNoveltyFeature, false);

    mLayoutFunc = [&](IGraphics *pGraphics) { SetupUI(pGraphics); };
}

void ReacomaExtension::OnUIClose() {
    SaveState();
    mGUIToggle = 0;
}

void ReacomaExtension::SetupUI(IGraphics *pGraphics) {
    // Clear previous controls
    pGraphics->RemoveAllControls();
    mProgressBar = nullptr;
    mCancelButton = nullptr;
    mAutoProcessButton = nullptr;
    mHasUserInteractedSinceLoad = false;
    mAutoProcessMode = false;

    const auto &theme = *mTheme; // Use the member theme object

    // Basic graphics setup
    pGraphics->EnableMouseOver(true);
    pGraphics->LoadFont("ibmplex", (void *)IBMPLEXMONO, IBMPLEXMONO_length);
    pGraphics->LoadFont("Roboto-Regular", (void *)ROBOTO_REGULAR,
                        ROBOTO_REGULAR_length);
    pGraphics->AttachPanelBackground(theme.bg);

    // --- Layout Constants ---
    const IRECT bounds = pGraphics->GetBounds();
    const float globalFramePadding = 15.f;
    const float verticalSpacing = 7.f;
    const float controlVisualHeight = 25.f;
    const float actionButtonHeight = 30.f;

    // --- Main Layout Areas ---
    IRECT mainContentArea = bounds.GetPadded(-globalFramePadding);
    mainContentArea.T += 5.f; // Top margin
    mainContentArea.B -= 5.f; // Bottom margin

    IRECT remainingArea = mainContentArea;
    IRECT bottomUtilityRowBounds =
        remainingArea.GetFromBottom(controlVisualHeight);
    remainingArea.B -= (bottomUtilityRowBounds.H() + verticalSpacing);
    IRECT actionButtonRowBounds =
        remainingArea.GetFromBottom(actionButtonHeight);
    remainingArea.B -= (actionButtonRowBounds.H() + verticalSpacing);
    IRECT currentLayoutBounds = remainingArea;

    // --- Algorithm Chooser Dropdown ---
    const float algoSelectorHeight = 60.f;
    if (currentLayoutBounds.H() >= algoSelectorHeight) {
        IRECT algorithmSelectorRect =
            currentLayoutBounds.GetFromTop(algoSelectorHeight);
        currentLayoutBounds.T = algorithmSelectorRect.B + verticalSpacing;

        const IVStyle menuButtonStyle =
            DEFAULT_STYLE.WithColor(kFG, theme.inactive)
                .WithColor(kBG, theme.bg)
                .WithColor(kPR, theme.inactive)
                .WithLabelText(theme.buttonStyle)
                .WithValueText(theme.buttonStyle)
                .WithDrawShadows(false);

        auto *pAlgoChooser = new IVButtonControl(
            algorithmSelectorRect,
            [this, pGraphics](IControl *pCaller) {
                SplashClickActionFunc(pCaller);
                static IPopupMenu menu{
                    "", {}, [this, pCaller](IPopupMenu *pMenu) {
                        int itemIndex = pMenu->GetChosenItemIdx();
                        if (itemIndex > -1) {
                            GetParam(kParamAlgorithmChoice)->Set(itemIndex);
                            auto selectedAlgo =
                                static_cast<EAlgorithmChoice>(itemIndex);
                            this->SetAlgorithmChoice(selectedAlgo, true);
                        }
                    }};

                menu.Clear();
                IParam *pAlgoParam = GetParam(kParamAlgorithmChoice);
                for (int i = 0; i <= pAlgoParam->GetMax(); ++i) {
                    menu.AddItem(pAlgoParam->GetDisplayTextAtIdx(i));
                }

                float x, y;
                pGraphics->GetMouseDownPoint(x, y);
                pGraphics->CreatePopupMenu(*pCaller, menu, x, y);
            },
            "", menuButtonStyle, false, true);

        pGraphics->AttachControl(pAlgoChooser, kCtrlTagAlgoChooser,
                                 "vcontrols");

        IParam *pAlgoParam = GetParam(kParamAlgorithmChoice);
        pAlgoChooser->SetValueStr(
            pAlgoParam->GetDisplayTextAtIdx(mCurrentAlgorithmChoice));
    }

    if (!mCurrentActiveAlgorithmPtr)
        return;

    // --- Algorithm Parameter Controls ---
    int numAlgoParams = mCurrentActiveAlgorithmPtr->GetNumAlgorithmParams();
    for (int i = 0; i < numAlgoParams; ++i) {
        if (currentLayoutBounds.H() < controlVisualHeight)
            break;

        int globalParamIdx = mCurrentActiveAlgorithmPtr->GetGlobalParamIdx(i);
        IParam *pParam = GetParam(globalParamIdx);
        IRECT controlCellRect =
            currentLayoutBounds.GetFromTop(controlVisualHeight);

        if (pParam->Type() == IParam::kTypeDouble ||
            pParam->Type() == IParam::kTypeInt) {
            pGraphics->AttachControl(new ReacomaParamTextControl(
                controlCellRect.GetVPadded(-5.f), globalParamIdx, theme));
        } else if (pParam->Type() == IParam::kTypeEnum &&
                   pParam->GetMax() > 0) {
            std::vector<std::string> labels;
            for (int val = 0; val <= pParam->GetMax(); ++val)
                labels.push_back(pParam->GetDisplayTextAtIdx(val));

            if (!labels.empty())
                pGraphics->AttachControl(new ReacomaSegmented(
                    controlCellRect, globalParamIdx, labels, theme));
        }

        currentLayoutBounds.T = controlCellRect.B + verticalSpacing;
    }

    // --- Action Buttons ---
    struct ButtonInfo {
        IActionFunction function;
        const char *label;
    };
    std::vector<ButtonInfo> buttonsToCreate;

    if (mCurrentActiveAlgorithmPtr->SupportsSegmentation()) {
        buttonsToCreate.push_back({ProcessAction<Mode::Segment>{}, "Segment"});
    }
    if (mCurrentActiveAlgorithmPtr->SupportsRegions()) {
        buttonsToCreate.push_back({ProcessAction<Mode::Regions>{}, "Regions"});
    }
    if (mCurrentActiveAlgorithmPtr->CreatesTakes()) {
        buttonsToCreate.push_back(
            {ProcessAction<Mode::ProcessAudio>{}, "Process"});
    }

    const int numActionButtons = buttonsToCreate.size();
    if (numActionButtons > 0) {
        for (int i = 0; i < numActionButtons; ++i) {
            const auto &buttonInfo = buttonsToCreate[i];
            IRECT b =
                actionButtonRowBounds.GetGridCell(0, i, 1, numActionButtons)
                    .GetHPadded(-theme.padding);
            pGraphics->AttachControl(new iplug::igraphics::ReacomaButton(
                b, buttonInfo.label, buttonInfo.function, theme));
        }
    }

    // --- Bottom Utility Row (Auto-Process, Progress Bar, Cancel) ---
    const float autoProcessControlWidth = 140.f;
    const float cancelButtonWidth = 80.f;

    IRECT paddedBottomRow = bottomUtilityRowBounds.GetHPadded(-theme.padding);

    IRECT autoProcessBounds =
        paddedBottomRow.GetFromLeft(autoProcessControlWidth);
    IRECT cancelBounds = paddedBottomRow.GetFromRight(cancelButtonWidth);
    IRECT progressBounds = paddedBottomRow; // Start with the full padded width
    progressBounds.L = autoProcessBounds.R + theme.padding * 5;
    progressBounds.R = cancelBounds.L - theme.padding * 5;

    mAutoProcessButton = new iplug::igraphics::ReacomaButton(
        autoProcessBounds, "",
        [this](IControl *p) {
            mAutoProcessMode = !mAutoProcessMode;
            UpdateAutoProcessButtonState();
        },
        theme);
    pGraphics->AttachControl(mAutoProcessButton);
    UpdateAutoProcessButtonState();

    mProgressBar =
        new ReacomaProgressBar(progressBounds, "Overall Progress", theme);
    pGraphics->AttachControl(mProgressBar);
    mProgressBar->SetDisabled(true);

    mCancelButton = new ReacomaButton(
        cancelBounds, "Cancel", [this](IControl *p) { CancelRunningJobs(); },
        theme);
    pGraphics->AttachControl(mCancelButton);
    mCancelButton->SetDisabled(true);
}

void ReacomaExtension::UpdateAutoProcessButtonState() {
    if (mAutoProcessButton) {
        if (mAutoProcessMode) {
            mAutoProcessButton->SetLabel("Auto-Process: ON");
        } else {
            mAutoProcessButton->SetLabel("Auto-Process: OFF");
        }
        mAutoProcessButton->SetToggleState(mAutoProcessMode);
    }
}

void ReacomaExtension::Process(Mode mode, bool force) {
    mCurrentProcessingMode = mode;
    mConcurrencyLimit = std::thread::hardware_concurrency();

    mPendingItemsQueue.clear();

    for (int i = 0; i < CountSelectedMediaItems(0); ++i) {
        mPendingItemsQueue.push_back(GetSelectedMediaItem(0, i));
    }

    mTotalBatchItems = mPendingItemsQueue.size();

    if (mPendingItemsQueue.empty() || mCurrentActiveAlgorithmPtr == nullptr) {
        return;
    }

    mIsProcessingBatch = true;
    mIsCancellationRequested = false;
    mLastReportedProgress = 0.0;
    mActiveJobs.clear();
    mFinalizationQueue.clear();

    if (mProgressBar) {
        mProgressBar->SetProgress(0.0);
        mProgressBar->SetDisabled(false);
    }
    if (mCancelButton) {
        mCancelButton->SetDisabled(false);
    }

    if (GetUI()) {
        IGraphics *pGraphics = GetUI();
        for (int i = 0; i < pGraphics->NControls(); ++i) {
            IControl *pControl = pGraphics->GetControl(i);
            if (pControl && pControl != mProgressBar &&
                pControl != mCancelButton) {
                pControl->SetDisabled(true);
            }
        }
    }

    mBatchUndoProject = GetItemProjectContext(mPendingItemsQueue.front());
    Undo_BeginBlock2(mBatchUndoProject);
}

void ReacomaExtension::OnParamChangeUI(int paramIdx, EParamSource source) {
    if (mAutoProcessMode && !mIsProcessingBatch &&
        mHasUserInteractedSinceLoad) {
        mProcessIsPending = true;
        mLastParamChangeTime = std::chrono::steady_clock::now();
    }

    SaveState();

    mHasUserInteractedSinceLoad = true;
}

void ReacomaExtension::OnIdle() {

    if (!mStateLoaded) {
        LoadState();
        SetAlgorithmChoice(static_cast<EAlgorithmChoice>(
                               GetParam(kParamAlgorithmChoice)->Int()),
                           true);
        mStateLoaded = true;
    }

    if (mUIRelayoutIsNeeded && GetUI()) {
        SetupUI(GetUI());
        mUIRelayoutIsNeeded = false;
        IGraphics *pGraphics = GetUI();
        if (pGraphics) {
            IControl *pControl =
                pGraphics->GetControlWithTag(kCtrlTagAlgoChooser);
            if (pControl) {
                IVButtonControl *pAlgoButton = pControl->As<IVButtonControl>();
                IParam *pAlgoParam = GetParam(kParamAlgorithmChoice);
                if (pAlgoButton && pAlgoParam) {
                    WDL_String currentDisplayText;
                    pAlgoParam->GetDisplay(currentDisplayText);
                    pAlgoButton->SetValueStr(currentDisplayText.Get());
                }
            }
        }
    }

    if (mProcessIsPending && !mIsProcessingBatch) {
        const auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - mLastParamChangeTime > AUTO_PROCESS_DELAY) {
            mProcessIsPending = false;

            Mode modeToRun = mCurrentActiveAlgorithmPtr &&
                                     mCurrentActiveAlgorithmPtr->CreatesTakes()
                                 ? Mode::ProcessAudio
                                 : Mode::Segment;

            if (mIsProcessingBatch) {
                CancelRunningJobs();
            }

            Process(modeToRun, true);
        }
    }

    if (!mIsProcessingBatch) {
        return;
    }

    if (mIsCancellationRequested) {
        for (auto &job : mActiveJobs) {
            job->Cancel();
            SetMediaItemInfo_Value(job->mItem, "C_LOCK", false);
        }

        mPendingItemsQueue.clear();
        mActiveJobs.clear();
        mFinalizationQueue.clear();

        mIsProcessingBatch = false;
        mIsCancellationRequested = false;

        Undo_EndBlock2(mBatchUndoProject, "Reacoma: Batch Process Cancelled",
                       -1);
        mBatchUndoProject = nullptr;

        ResetUIState();
        UpdateArrange();
        UpdateTimeline();
        return;
    }

    for (auto it = mActiveJobs.begin(); it != mActiveJobs.end();) {
        if ((*it)->IsFinished()) {
            mFinalizationQueue.push_back(std::move(*it));
            it = mActiveJobs.erase(it);
        } else {
            ++it;
        }
    }

    while (mActiveJobs.size() < mConcurrencyLimit &&
           !mPendingItemsQueue.empty()) {
        MediaItem *itemToProcess = mPendingItemsQueue.front();
        mPendingItemsQueue.pop_front();

        auto job =
            ProcessingJob::Create(mCurrentAlgorithmChoice, itemToProcess, this);
        if (job) {
            job->Start();
            mActiveJobs.push_back(std::move(job));
        }
    }

    if (!mFinalizationQueue.empty()) {
        auto &finishedJob = mFinalizationQueue.front();
        finishedJob->Finalize();
        mFinalizationQueue.pop_front();
    }

    if (mProgressBar && mTotalBatchItems > 0) {
        double totalProgressUnits = 0.0;

        for (const auto &job : mActiveJobs) {
            totalProgressUnits += job->GetProgress();
        }

        size_t completedJobs =
            mTotalBatchItems - mPendingItemsQueue.size() - mActiveJobs.size();
        totalProgressUnits += static_cast<double>(completedJobs);

        double overallProgress = totalProgressUnits / mTotalBatchItems;
        if (overallProgress > mLastReportedProgress) {
            mLastReportedProgress = overallProgress;
        }
        mProgressBar->SetProgress(mLastReportedProgress);
    }

    if (mPendingItemsQueue.empty() && mActiveJobs.empty() &&
        mFinalizationQueue.empty()) {
        mIsProcessingBatch = false;
        Undo_EndBlock2(mBatchUndoProject, "Reacoma: Process Batch", -1);
        mBatchUndoProject = nullptr;

        ResetUIState();
        UpdateArrange();
        UpdateTimeline();
    }
}

void ReacomaExtension::SetAlgorithmChoice(EAlgorithmChoice choice,
                                          bool triggerUIRelayout) {
    mCurrentAlgorithmChoice = choice;
    switch (choice) {
        case kNoveltySlice:
            mCurrentActiveAlgorithmPtr = mNoveltyAlgorithm.get();
            break;
        case kHPSS:
            mCurrentActiveAlgorithmPtr = mHPSSAlgorithm.get();
            break;
        case kNMF:
            mCurrentActiveAlgorithmPtr = mNMFAlgorithm.get();
            break;
        case kOnsetSlice:
            mCurrentActiveAlgorithmPtr = mOnsetSliceAlgorithm.get();
            break;
        case kTransientSlice:
            mCurrentActiveAlgorithmPtr = mTransientSliceAlgorithm.get();
            break;
        case kTransients:
            mCurrentActiveAlgorithmPtr = mTransientsAlgorithm.get();
            break;
        case kAmpGate:
            mCurrentActiveAlgorithmPtr = mAmpGateAlgorithm.get();
            break;
        case kNoveltyFeature:
            mCurrentActiveAlgorithmPtr = mNoveltyFeatureAlgorithm.get();
            break;
        default:
            mCurrentActiveAlgorithmPtr = nullptr;
            break;
    }
    if (triggerUIRelayout) {
        mUIRelayoutIsNeeded = true;
    }
}

void ReacomaExtension::CancelRunningJobs() {
    if (mIsProcessingBatch) {
        mIsCancellationRequested = true;
        if (mCancelButton) {
            mCancelButton->SetDisabled(true);
        }
    }
}

void ReacomaExtension::ResetUIState() {
    if (GetUI()) {
        IGraphics *pGraphics = GetUI();
        for (int i = 0; i < pGraphics->NControls(); ++i) {
            IControl *pControl = pGraphics->GetControl(i);
            if (pControl) {
                pControl->SetDisabled(false);
            }
        }
    }

    if (mProgressBar) {
        mProgressBar->SetDisabled(true);
        mProgressBar->SetProgress(0.0);
    }

    if (mCancelButton) {
        mCancelButton->SetDisabled(true);
    }
}

std::string ReacomaExtension::GetSettingsFilePath() const {
    const char *resourcePath = GetResourcePath();
    if (resourcePath && strlen(resourcePath) > 0) {
        std::string path(resourcePath);
        path += "/reacoma-settings.ini";
        return path;
    }
    return "";
}

void ReacomaExtension::SaveState() {
    std::string path = GetSettingsFilePath();
    if (path.empty())
        return;

    FILE *file = fopen(path.c_str(), "w");
    if (!file)
        return;

    // Save the global algorithm choice parameter
    IParam *pChoiceParam = GetParam(kParamAlgorithmChoice);
    if (pChoiceParam && pChoiceParam->GetName()) {
        fprintf(file, "%s=%f\n", pChoiceParam->GetName(),
                pChoiceParam->GetNormalized());
    }

    // Save all algorithm-specific parameters
    for (IAlgorithm *pAlgorithm : mAllAlgorithms) {
        if (!pAlgorithm || !pAlgorithm->GetName())
            continue;

        for (int i = 0; i < pAlgorithm->GetNumAlgorithmParams(); ++i) {
            int globalIdx = pAlgorithm->GetGlobalParamIdx(i);
            IParam *pParam = GetParam(globalIdx);

            if (!pParam || !pParam->GetName())
                continue;

            std::string algoNameStr = pAlgorithm->GetName();
            std::string paramNameStr = pParam->GetName();

            std::replace(algoNameStr.begin(), algoNameStr.end(), ' ', '_');
            std::replace(paramNameStr.begin(), paramNameStr.end(), ' ', '_');

            if (algoNameStr.empty() || paramNameStr.empty())
                continue;

            fprintf(file, "%s:%s=%f\n", algoNameStr.c_str(),
                    paramNameStr.c_str(), pParam->GetNormalized());
        }
    }
    fclose(file);
}

void ReacomaExtension::LoadState() {
    std::string path = GetSettingsFilePath();
    if (path.empty())
        return;

    std::ifstream settingsFile(path);
    if (!settingsFile.is_open())
        return;

    std::map<std::string, double> loadedSettings;
    std::string line;
    while (std::getline(settingsFile, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string valueStr = line.substr(pos + 1);
            try {
                double value = std::stod(valueStr);
                loadedSettings[key] = value;
            } catch (const std::exception &e) {
                // Ignore parsing errors
            }
        }
    }

    // Load global algorithm choice
    IParam *pChoiceParam = GetParam(kParamAlgorithmChoice);
    if (pChoiceParam && pChoiceParam->GetName() &&
        loadedSettings.count(pChoiceParam->GetName())) {
        pChoiceParam->SetNormalized(loadedSettings[pChoiceParam->GetName()]);
    }

    // Load algorithm-specific parameters
    for (IAlgorithm *pAlgorithm : mAllAlgorithms) {
        if (!pAlgorithm || !pAlgorithm->GetName())
            continue;

        for (int i = 0; i < pAlgorithm->GetNumAlgorithmParams(); ++i) {
            int globalIdx = pAlgorithm->GetGlobalParamIdx(i);
            IParam *pParam = GetParam(globalIdx);
            if (!pParam || !pParam->GetName())
                continue;

            std::string algoName = pAlgorithm->GetName();
            std::string paramName = pParam->GetName();

            // Create the same safe key that was used for saving
            std::replace(algoName.begin(), algoName.end(), ' ', '_');
            std::replace(paramName.begin(), paramName.end(), ' ', '_');

            std::string key = algoName + ":" + paramName;
            if (loadedSettings.count(key)) {
                pParam->SetNormalized(loadedSettings[key]);
            }
        }
    }
}
