// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/BlackboardRegistry.h"

#include <cstddef>

#include <catch.hpp>

#define for_each_blackboard(x)                  \
    for (*x = 0; *x < numBlackboards; ++(*x))

using BlackboardRegistry = blackboard::BlackboardRegistry;
using Blackboard = blackboard::Blackboard;

constexpr std::size_t numBlackboards = 100;
static Blackboard* blackboards[numBlackboards] = {nullptr};

TEST_CASE("CreateGetAndDestroyBlackboard", "[BlackboardRegistryTest]") {
    std::size_t blackboardId;

    BlackboardRegistry::SingletonCreate();
    BlackboardRegistry* blackboardRegistry = &blackboard::GetBlackboardRegistry();

    for_each_blackboard(&blackboardId) {
        blackboards[blackboardId] =
            &blackboardRegistry->CreateBlackboard("Blackboard#" + std::to_string(blackboardId));
        REQUIRE(blackboards[blackboardId] != nullptr);
    }

    for_each_blackboard(&blackboardId) {
        REQUIRE(blackboardRegistry->GetBlackboard(
            "Blackboard#" + std::to_string(blackboardId)) == blackboards[blackboardId]);
    }

    for_each_blackboard(&blackboardId) {
        blackboardRegistry->DestroyBlackboard("Blackboard#" + std::to_string(blackboardId));
    }

    for_each_blackboard(&blackboardId) {
        REQUIRE(blackboardRegistry->GetBlackboard(
            "Blackboard#" + std::to_string(blackboardId)) == nullptr);
    }

    BlackboardRegistry::SingletonDestroy();
}
