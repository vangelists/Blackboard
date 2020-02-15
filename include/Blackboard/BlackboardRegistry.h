// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#pragma once

#include "Blackboard/Blackboard.h"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace blackboard {

class BlackboardRegistry final {

public:
    using BlackboardId = std::string_view;

    BlackboardRegistry(BlackboardRegistry& from) = delete;
    BlackboardRegistry& operator=(const BlackboardRegistry& from) = delete;

    Blackboard* GetBlackboard(BlackboardId blackboardId);
    Blackboard& CreateBlackboard(BlackboardId blackboardId);
    void DestroyBlackboard(BlackboardId blackboardId);

    static void SingletonCreate();
    static void SingletonDestroy();
    static BlackboardRegistry& SingletonGet();

private:
    using Blackboards = std::map<std::string, std::unique_ptr<Blackboard>, std::less<>>;

    BlackboardRegistry();
    ~BlackboardRegistry();

    Blackboards blackboards;
    std::mutex blackboardsMutex;

    static inline BlackboardRegistry* singleton = nullptr;
};


BlackboardRegistry& GetBlackboardRegistry();

} // namespace blackboard
