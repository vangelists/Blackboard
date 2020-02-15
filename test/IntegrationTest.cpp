// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/BlackboardRegistry.h"
#include "Blackboard/Object.h"
#include "Blackboard/Value.h"

#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <cassert>
#include <cstddef>

#include <catch.hpp>

#define for_each_blackboard(x)                  \
    for (*x = 0; *x < numBlackboards; ++(*x))

using BlackboardRegistry = blackboard::BlackboardRegistry;
using Blackboard = blackboard::Blackboard;
using Object = blackboard::Object;
using Value = blackboard::Value;
using Event = Blackboard::Event;
using EventID = Blackboard::EventID;
using EventHandler = Blackboard::EventHandler;
using EventHandlerUniqueId = blackboard::EventHandlerUniqueId;
using CallEventHandlerOnce = blackboard::Blackboard::CallEventHandlerOnce;

using namespace std::string_literals;

constexpr std::size_t numThreads = 5;
constexpr std::size_t numSubthreads = 250;
constexpr std::size_t numBlackboards = 5;

struct BlackboardsInfo {
    Blackboard* blackboards[numBlackboards];
    std::thread threads[numBlackboards][numSubthreads + 1];
};

static std::map<std::thread::id, BlackboardsInfo> threadInfo;

#ifdef PRINT_THREAD_INFO
static std::mutex ioMutex;
#endif // PRINT_THREAD_INFO

static size_t timesSampleEventHandlerCalled = 0;

static std::mutex stringMutex;
static std::mutex threadInfoMutex;
static std::mutex timesSampleEventHandlerCalledMutex;

static Event dummyEvent = "dummyEvent";
static Object dummyObject{};
static Object* eventContent;

static Value numberValueKey{"numberValueKey"s};
static Value booleanValueKey{"booleanValueKey"s};
static Value referenceValueKey{"referenceValueKey"s};
static Value stringValueKey{"stringValueKey"s};
static Value objectValueKey{"objectValueKey"s};

static Event sampleEvent = "sampleEvent";

static EventHandler sampleEventHandler = [](EventID, const Object&) {
    const std::lock_guard<std::mutex> lock(timesSampleEventHandlerCalledMutex);
    ++timesSampleEventHandlerCalled;
    return true;
};

static std::string generateBlackboardId(const std::string& threadId, std::size_t blackboardId) {
    std::lock_guard<std::mutex> lock(stringMutex);
    return threadId + "#" + std::to_string(blackboardId);
}

static std::string threadIdToString(const std::thread::id threadId) {
    std::stringstream stringStream;
    stringStream << threadId;
    return stringStream.str();
}

static BlackboardsInfo& getBlackboardsInfo(const std::thread::id threadId) {
    std::lock_guard<std::mutex> lock(threadInfoMutex);
    if (const auto& thisThreadInfo = threadInfo.find(threadId); thisThreadInfo == threadInfo.end()) {
        threadInfo.emplace(threadId, BlackboardsInfo());
    }
    return threadInfo[threadId];
}

static void verifyEventContent(const Object& eventContent) {
    assert(eventContent.GetValue(numberValueKey));
    assert(eventContent.GetValue(numberValueKey)->ToNumber() == 3.14);

    assert(eventContent.GetValue(booleanValueKey));
    assert(eventContent.GetValue(booleanValueKey)->ToBoolean());

    assert(eventContent.GetValue(referenceValueKey));
    assert(eventContent.GetValue(referenceValueKey)->ToReference() == static_cast<void *>(&dummyObject));

    assert(eventContent.GetValue(stringValueKey));
    assert(eventContent.GetValue(stringValueKey)->ToString() == "stringValue");

    assert(eventContent.GetValue(objectValueKey));
    assert(eventContent.GetValue(objectValueKey)->ToObject() == dummyObject);
    assert(eventContent.GetValue(objectValueKey)->ToObject().GetValue(stringValueKey)->ToString() == "stringValueKey");
}

static void initializeTest() {
    Value numberValue{3.14};
    Value booleanValue{true};
    Value referenceValue{static_cast<void *>(&dummyObject)};
    Value stringValue{"stringValue"s};

    dummyObject.AddValue(stringValueKey, stringValueKey);
    Value objectValue{dummyObject};

    eventContent = new Object();
    eventContent->AddValue(numberValueKey, numberValue);
    eventContent->AddValue(booleanValueKey, booleanValue);
    eventContent->AddValue(referenceValueKey, referenceValue);
    eventContent->AddValue(stringValueKey, stringValue);
    eventContent->AddValue(objectValueKey, objectValue);
}

static void runBlackboardTest(Blackboard* blackboard) {
    blackboard->PostEvent(sampleEvent, *eventContent);
    blackboard->PostQueuedEvent(sampleEvent, *eventContent);
    blackboard->ProcessQueuedEvents();
}

static void prepareBlackboard(Blackboard* blackboard) {
    EventHandler dummyHandler01 = [=](EventID, const Object& eventContent) {
        verifyEventContent(eventContent);
        runBlackboardTest(blackboard);
        return true;
    };
    EventHandler dummyHandler02 = [=](EventID, const Object& eventContent) {
        verifyEventContent(eventContent);
        runBlackboardTest(blackboard);
        return true;
    };
    EventHandler dummyHandler03 = [=](EventID, const Object& eventContent) {
        verifyEventContent(eventContent);
        runBlackboardTest(blackboard);
        return true;
    };
    EventHandler oneTimeStopsInvocationLoop = [=](EventID, const Object& eventContent) {
        verifyEventContent(eventContent);
        runBlackboardTest(blackboard);
        blackboard->StopInvocationLoop();
        return true;
    };

    assert(blackboard->AddEventHandler(dummyEvent, dummyHandler01, CallEventHandlerOnce::No));
    assert(blackboard->AddEventHandler(dummyEvent, dummyHandler02, CallEventHandlerOnce::No));
    assert(blackboard->AddEventHandler(dummyEvent, dummyHandler03, CallEventHandlerOnce::No));
    assert(blackboard->AddEventHandler(dummyEvent, oneTimeStopsInvocationLoop,
                                       CallEventHandlerOnce::Yes));
    assert(blackboard->AddEventHandler(sampleEvent, sampleEventHandler, CallEventHandlerOnce::No));

    blackboard->PostEvent(dummyEvent, *eventContent);
    blackboard->ClearEventHandlers(dummyEvent);

    assert(blackboard->AddEventHandler(dummyEvent, dummyHandler01, CallEventHandlerOnce::No));
    assert(blackboard->AddEventHandler(dummyEvent, dummyHandler02, CallEventHandlerOnce::No));
    assert(blackboard->AddEventHandler(dummyEvent, dummyHandler03, CallEventHandlerOnce::No));
    assert(blackboard->AddEventHandler(dummyEvent, oneTimeStopsInvocationLoop,
                                       CallEventHandlerOnce::Yes));
}

static void runBlackboardRegistryTest(BlackboardRegistry* blackboardRegistry, std::size_t threadId) {
    std::size_t blackboardId;

    const auto thisThreadId = std::this_thread::get_id();
    const auto thisThreadIdString = threadIdToString(thisThreadId);

    auto& blackboardsInfo = getBlackboardsInfo(thisThreadId);

    for_each_blackboard(&blackboardId) {
        blackboardsInfo.blackboards[blackboardId] = &blackboardRegistry->CreateBlackboard(
            generateBlackboardId(thisThreadIdString, blackboardId));
        assert(blackboardsInfo.blackboards[blackboardId]);
    }

    for_each_blackboard(&blackboardId) {
        assert(blackboardRegistry->GetBlackboard(
            generateBlackboardId(thisThreadIdString, blackboardId)) ==
                                 blackboardsInfo.blackboards[blackboardId]);
    }

    for_each_blackboard(&blackboardId) {
        prepareBlackboard(blackboardsInfo.blackboards[blackboardId]);

        for (std::size_t subthreadId = 0; subthreadId < numSubthreads; ++subthreadId) {
            blackboardsInfo.threads[blackboardId][subthreadId] =
                std::thread(runBlackboardTest, blackboardsInfo.blackboards[blackboardId]);

        #ifdef PRINT_THREAD_INFO
            {
                auto currentlyRunningThreadId =
                        blackboardsInfo.threads[blackboardId][subthreadId].get_id();
                auto currentlyRunningThreadIdString = threadIdToString(currentlyRunningThreadId);

                std::lock_guard<std::mutex> ioMutexGuard(ioMutex);

                std::cout << "Thread " << thisThreadIdString << "\n    Blackboard "
                          << generateBlackboardId(thisThreadIdString, blackboardId)
                          << " - Subthread " << currentlyRunningThreadIdString << blackboardId
                          << std::endl;
            }
        #endif // PRINT_THREAD_INFO
        }

        for (std::size_t subthreadId = 0; subthreadId < numSubthreads; ++subthreadId) {
            if (blackboardsInfo.threads[blackboardId][subthreadId].joinable()) {
                blackboardsInfo.threads[blackboardId][subthreadId].join();
            }
        }
    }

    for_each_blackboard(&blackboardId) {
        blackboardRegistry->DestroyBlackboard(generateBlackboardId(thisThreadIdString,
                                                                   blackboardId));
    }

    for_each_blackboard(&blackboardId) {
        assert(blackboardRegistry->GetBlackboard(generateBlackboardId(thisThreadIdString,
                                                                      blackboardId)) == nullptr);
    }
}


TEST_CASE("IntegrationTest", "[IntegrationTest]") {
    BlackboardRegistry::SingletonCreate();
    BlackboardRegistry* blackboardRegistry = &blackboard::GetBlackboardRegistry();

    std::thread threads[numThreads];

    initializeTest();

    for (std::size_t threadId = 0; threadId < numThreads; ++threadId) {
        threads[threadId] = std::thread(runBlackboardRegistryTest, blackboardRegistry, threadId);
    }

    for (std::size_t threadId = 0; threadId < numThreads; ++threadId) {
        if (threads[threadId].joinable()) {
            threads[threadId].join();
        }
    }

    assert(timesSampleEventHandlerCalled == (numThreads * numBlackboards) * (2 * numSubthreads + 8));

    delete eventContent;
    BlackboardRegistry::SingletonDestroy();
}
