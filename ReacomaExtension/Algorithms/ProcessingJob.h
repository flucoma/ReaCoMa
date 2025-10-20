#pragma once

#include <memory>
#include "ReacomaExtension.h"
#include "IAlgorithm.h"
#include "NoveltySliceAlgorithm.h"
#include "HPSSAlgorithm.h"
#include "NMFAlgorithm.h"
#include "OnsetSliceAlgorithm.h"
#include "TransientSliceAlgorithm.h"
#include "TransientAlgorithm.h"
#include "AmpGateAlgorithm.h"

class MediaItem;
class ReacomaExtension;

class ProcessingJob {
public:
    static std::unique_ptr<ProcessingJob>
    Create(ReacomaExtension::EAlgorithmChoice algoChoice, MediaItem *item,
           ReacomaExtension *provider);

    void Start();
    bool IsFinished();
    void Finalize();
    void Cancel();

    double GetProgress() { return mAlgorithm->GetProgress(); }

    std::unique_ptr<IAlgorithm> mAlgorithm;
    MediaItem *mItem;

    ProcessingJob(std::unique_ptr<IAlgorithm> algorithm, MediaItem *item);
};
