#include "IAlgorithm.h"

IAlgorithm::IAlgorithm(ReacomaExtension *apiProvider)
    : mApiProvider(apiProvider) {}

IAlgorithm::~IAlgorithm() = default;
