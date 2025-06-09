#pragma once
#include <memory>
#include <vector>

class MediaItem;
class ReacomaExtension;

class IAlgorithm {
  public:
    IAlgorithm(ReacomaExtension *apiProvider);
    virtual ~IAlgorithm();

    virtual bool StartProcessItemAsync(MediaItem *item) = 0;
    virtual bool IsFinished() = 0;
    virtual bool FinalizeProcess(MediaItem *item) = 0;

    virtual double GetProgress() = 0;
    virtual void Cancel() = 0;

    virtual const char *GetName() const = 0;
    virtual void RegisterParameters() = 0;
    int GetGlobalParamIdx(int algorithmParamEnum) const {
        return mBaseParamIdx + algorithmParamEnum;
    }
    virtual int GetNumAlgorithmParams() const = 0;
    int GetBaseParamIdx() const { return mBaseParamIdx; }
    void SetBaseParamIdx(int idx) { mBaseParamIdx = idx; }

    virtual bool SupportsSegmentation() = 0;
    virtual bool SupportsRegions() = 0;
    virtual bool CreatesTakes() = 0;

  protected:
    ReacomaExtension *mApiProvider;
    int mBaseParamIdx = 0;
};
