// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/BlackboardRegistry.h"

#include <cassert>

namespace blackboard {

BlackboardRegistry::BlackboardRegistry() = default;

BlackboardRegistry::~BlackboardRegistry() = default;

//--------------------------------------------------------------------------------------------------

void BlackboardRegistry::SingletonCreate() {
    if (!singleton) {
        singleton = new BlackboardRegistry;
    }
}

void BlackboardRegistry::SingletonDestroy() {
    if (singleton) {
        delete singleton;
        singleton = nullptr;
    }
}

BlackboardRegistry& BlackboardRegistry::SingletonGet() {
    assert(singleton);
    return *singleton;
}

BlackboardRegistry &GetBlackboardRegistry() {
    return BlackboardRegistry::SingletonGet();
}

//--------------------------------------------------------------------------------------------------

Blackboard* BlackboardRegistry::GetBlackboard(BlackboardId blackboardId) {
    const std::lock_guard<std::mutex> lock(blackboardsMutex);
    const auto& keyValuePair = blackboards.find(blackboardId);
    return keyValuePair != blackboards.end() ? &*keyValuePair->second : nullptr;
}

Blackboard& BlackboardRegistry::CreateBlackboard(BlackboardId blackboardId) {
    const std::lock_guard<std::mutex> lock(blackboardsMutex);

    if (const auto& keyValuePair = blackboards.find(blackboardId); keyValuePair != blackboards.end()) {
        Blackboard& blackboard = *keyValuePair->second;
        return blackboard;
    }

    const auto& keyValuePair = blackboards.emplace(std::piecewise_construct,
                                                   std::forward_as_tuple(blackboardId),
                                                   std::forward_as_tuple(std::make_unique<Blackboard>()));
    return *keyValuePair.first->second;
}

void BlackboardRegistry::DestroyBlackboard(BlackboardId blackboardId) {
    const std::lock_guard<std::mutex> lock(blackboardsMutex);
    if (const auto& blackboard = blackboards.find(blackboardId); blackboard != blackboards.end()) {
        blackboards.erase(blackboard);
    }
}

} // namespace blackboard
