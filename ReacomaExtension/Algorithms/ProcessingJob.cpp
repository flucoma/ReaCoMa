#include "ProcessingJob.h"
#include "NoveltySliceAlgorithm.h"
#include "HPSSAlgorithm.h"
#include "NMFAlgorithm.h"
#include "OnsetSliceAlgorithm.h"
#include "ReacomaExtension.h"
#include "TransientSliceAlgorithm.h"
#include "TransientAlgorithm.h"
#include "NoveltyFeatureAlgorithm.h"
#include "AmpSliceAlgorithm.h"
#include <memory>

ProcessingJob::ProcessingJob(std::unique_ptr<IAlgorithm> algorithm,
                             MediaItem *item)
    : mAlgorithm(std::move(algorithm)), mItem(item) {}

void ProcessingJob::Start() {
    if (mAlgorithm && mItem) {
        mAlgorithm->StartProcessItemAsync(mItem);
    }
}

bool ProcessingJob::IsFinished() {
    return mAlgorithm ? mAlgorithm->IsFinished() : true;
}

void ProcessingJob::Finalize() {
    if (mAlgorithm && mItem) {
        mAlgorithm->FinalizeProcess(mItem);
    }
}

void ProcessingJob::Cancel() {
    if (mAlgorithm) {
        mAlgorithm->Cancel();
    }
}

std::unique_ptr<ProcessingJob>
ProcessingJob::Create(ReacomaExtension::EAlgorithmChoice algoChoice,
                      MediaItem *item, ReacomaExtension *provider) {
    std::unique_ptr<IAlgorithm> algorithm = nullptr;
    const IAlgorithm *prototypeAlgorithm = nullptr;

    switch (algoChoice) {
        case ReacomaExtension::kNoveltySlice:
            algorithm = std::make_unique<NoveltySliceAlgorithm>(provider);
            prototypeAlgorithm = provider->GetNoveltySliceAlgorithm();
            break;
        case ReacomaExtension::kHPSS:
            algorithm = std::make_unique<HPSSAlgorithm>(provider);
            prototypeAlgorithm = provider->GetHPSSAlgorithm();
            break;
        case ReacomaExtension::kNMF:
            algorithm = std::make_unique<NMFAlgorithm>(provider);
            prototypeAlgorithm = provider->GetNMFAlgorithm();
            break;
        case ReacomaExtension::kOnsetSlice:
            algorithm = std::make_unique<OnsetSliceAlgorithm>(provider);
            prototypeAlgorithm = provider->GetOnsetSliceAlgorithm();
            break;
        case ReacomaExtension::kTransients:
            algorithm = std::make_unique<TransientAlgorithm>(provider);
            prototypeAlgorithm = provider->GetTransientsAlgorithm();
            break;
        case ReacomaExtension::kTransientSlice:
            algorithm = std::make_unique<TransientSliceAlgorithm>(provider);
            prototypeAlgorithm = provider->GetTransientSliceAlgorithm();
            break;
        case ReacomaExtension::kAmpGate:
            algorithm = std::make_unique<AmpGateAlgorithm>(provider);
            prototypeAlgorithm = provider->GetAmpGateAlgorithm();
            break;
        case ReacomaExtension::kAmpSlice:
            algorithm = std::make_unique<AmpSliceAlgorithm>(provider);
            prototypeAlgorithm = provider->GetAmpSliceAlgorithm();
            break;
    }

    if (algorithm && prototypeAlgorithm) {
        algorithm->SetBaseParamIdx(prototypeAlgorithm->GetBaseParamIdx());
        return std::make_unique<ProcessingJob>(std::move(algorithm), item);
    }
    return nullptr;
}
