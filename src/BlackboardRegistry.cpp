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
    const auto& iterator = blackboards.find(blackboardId);
    return iterator != blackboards.end() ? &*iterator->second : nullptr;
}

Blackboard& BlackboardRegistry::CreateBlackboard(BlackboardId blackboardId) {
    const std::lock_guard<std::mutex> lock(blackboardsMutex);

    if (const auto& iterator = blackboards.find(blackboardId); iterator != blackboards.end()) {
        const auto& [_, blackboard] = *iterator;
        return *blackboard;
    }

    const auto& [iterator, success] =
            blackboards.emplace(std::piecewise_construct,
                                std::forward_as_tuple(blackboardId),
                                std::forward_as_tuple(std::make_unique<Blackboard>()));
    if (!success) {
      throw std::bad_alloc();
    }

    return *iterator->second;
}

void BlackboardRegistry::DestroyBlackboard(BlackboardId blackboardId) {
    const std::lock_guard<std::mutex> lock(blackboardsMutex);
    if (const auto& blackboard = blackboards.find(blackboardId); blackboard != blackboards.end()) {
        blackboards.erase(blackboard);
    }
}

} // namespace blackboard
