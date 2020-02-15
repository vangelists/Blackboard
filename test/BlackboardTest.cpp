// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/Blackboard.h"
#include "Blackboard/Object.h"
#include "Blackboard/Value.h"

#include <cstddef>

#include <catch.hpp>

using namespace std::string_literals;
using namespace blackboard;

using Event = Blackboard::Event;
using EventID = Blackboard::EventID;
using EventHandler = Blackboard::EventHandler;
using CallEventHandlerOnce = Blackboard::CallEventHandlerOnce;
using BlackboardException = Blackboard::BlackboardException;
using BlackboardQueuedException = Blackboard::BlackboardQueuedException;
using UnhandledEventException = Blackboard::UnhandledEventException;

//--------------------------------------------------------------------------------------------------

#define EVENT_ID_DEFINITION(x)      static Event event##x = #x;
#define EVENT_STATUS_DEFINITION(x)  static bool x##HandlerCalled = false;
#define EVENT_CONTENT_DEFINITION(x) static const Object* x##EventContent = nullptr;

#define EVENT_HANDLER_DEFINITION(x)                                         \
    static bool x##Handler(EventID eventId, const Object& eventContent) {   \
        REQUIRE(eventId == #x);                                             \
        x##EventContent = &eventContent;                                    \
        x##HandlerCalled = true;                                            \
        return true;                                                        \
}

#define EVENT_DEFINITION(x)     \
    EVENT_ID_DEFINITION(x)      \
    EVENT_STATUS_DEFINITION(x)  \
    EVENT_CONTENT_DEFINITION(x) \
    EVENT_HANDLER_DEFINITION(x)

//--------------------------------------------------------------------------------------------------

EVENT_DEFINITION(MouseClickLeft)
EVENT_DEFINITION(MouseClickMiddle)
EVENT_DEFINITION(MouseClickRight)


static bool MouseEventHandlerCalled = false;

static bool MouseEventHandler(EventID eventId, const Object&) {
    REQUIRE((eventId == eventMouseClickLeft || eventId == eventMouseClickMiddle ||
             eventId == eventMouseClickRight));
    MouseEventHandlerCalled = true;
    return true;
}

//--------------------------------------------------------------------------------------------------

TEST_CASE("AddEventHandler", "[BlackboardTest]") {
    Blackboard blackboard;

    // Register event handlers.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseClickMiddleHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::No);

    // Create dummy event content.
    Object dummyObject{};

    // Post events and verify that the corresponding handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickMiddle, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;

    // Clear event handlers for left and right mouse click.
    blackboard.ClearEventHandlers(eventMouseClickLeft);
    blackboard.ClearEventHandlers(eventMouseClickRight);

    // Register persistent event handlers for left and right mouse click.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    // Register temporary event handlers for left and right mouse click.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::Yes);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::Yes);

    // Post events and verify that the corresponding handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;

    // Post events again and verify that only the persistent event handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
}

TEST_CASE("RemoveEventHandler", "[BlackboardTest]") {
    Blackboard blackboard;

    // Register event handlers.
    std::size_t mouseClickLeftHandlerId = blackboard.AddEventHandler(eventMouseClickLeft,
                                                                     MouseClickLeftHandler,
                                                                     CallEventHandlerOnce::No);
    std::size_t mouseClickMiddleHandlerId = blackboard.AddEventHandler(eventMouseClickMiddle,
                                                                       MouseClickMiddleHandler,
                                                                       CallEventHandlerOnce::No);
    std::size_t mouseClickRightHandlerId = blackboard.AddEventHandler(eventMouseClickRight,
                                                                      MouseClickRightHandler,
                                                                      CallEventHandlerOnce::No);

    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::No);

    // Create dummy event content.
    Object dummyObject{};

    // Post events and verify that the corresponding handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickMiddle, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;

    // Remove specialized event handlers.
    blackboard.RemoveEventHandler(eventMouseClickLeft, mouseClickLeftHandlerId);
    blackboard.RemoveEventHandler(eventMouseClickMiddle, mouseClickMiddleHandlerId);
    blackboard.RemoveEventHandler(eventMouseClickRight, mouseClickRightHandlerId);

    // Post events and verify that only the generic handler has been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(!MouseClickLeftHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickMiddle, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(!MouseClickMiddleHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(!MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
}

TEST_CASE("SelfRemovingEventHandler", "[BlackboardTest]") {
    Blackboard blackboard;

    // Create self-removing event handler.
    EventHandlerUniqueId eventHandlerId = 0;
    EventHandler eventHandler = [&blackboard, &eventHandlerId](EventID, const Object&) {
        MouseClickRightHandlerCalled = true;
        blackboard.RemoveEventHandler(eventMouseClickRight, eventHandlerId);
        return true;
    };

    // Create dummy handlers.
    EventHandler mouseEventHandler = [](EventID, const Object&) {
        MouseEventHandlerCalled = true;
        return true;
    };
    EventHandler mouseClickLeftEventHandler = [](EventID, const Object&) {
        MouseClickLeftHandlerCalled = true;
        return true;
    };
    EventHandler mouseClickMiddleEventHandler = [](EventID, const Object&) {
        MouseClickMiddleHandlerCalled = true;
        return true;
    };

    // Register handlers.
    blackboard.AddEventHandler(eventMouseClickRight, mouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, mouseClickLeftEventHandler,
                               CallEventHandlerOnce::No);
    eventHandlerId = blackboard.AddEventHandler(eventMouseClickRight, eventHandler,
                                                CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, mouseClickMiddleEventHandler,
                               CallEventHandlerOnce::No);

    // Create dummy event content.
    Object dummyObject{};

    // Call handlers.
    blackboard.PostEvent(eventMouseClickRight, dummyObject);

    // Make sure the handlers have been successfully called.
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;
    MouseClickRightHandlerCalled = false;

    // Make sure the self-removing handler has been successfully removed.
    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    REQUIRE(!MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
}

TEST_CASE("SelfRemovingQueuedEventHandler", "[BlackboardTest]") {
    Blackboard blackboard;

    // Create self-removing event handler.
    EventHandlerUniqueId queuedEventHandlerId = 0;
    EventHandler queuedEventHandler = [&](EventID, const Object&) {
        MouseClickRightHandlerCalled = true;
        blackboard.RemoveEventHandler(eventMouseClickRight, queuedEventHandlerId);
        return true;
    };

    // Create dummy handlers.
    EventHandler mouseEventHandler = [](EventID, const Object&) {
        MouseEventHandlerCalled = true;
        return true;
    };
    EventHandler mouseClickLeftEventHandler = [](EventID, const Object&) {
        MouseClickLeftHandlerCalled = true;
        return true;
    };
    EventHandler mouseClickMiddleEventHandler = [](EventID, const Object&) {
        MouseClickMiddleHandlerCalled = true;
        return true;
    };

    // Register handlers.
    blackboard.AddEventHandler(eventMouseClickRight, mouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, mouseClickLeftEventHandler,
                               CallEventHandlerOnce::No);
    queuedEventHandlerId = blackboard.AddEventHandler(eventMouseClickRight, queuedEventHandler,
                                                      CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, mouseClickMiddleEventHandler,
                               CallEventHandlerOnce::No);

    // Create dummy event content.
    Object dummyObject{};

    // Post queued event.
    blackboard.PostQueuedEvent(eventMouseClickRight, dummyObject);

    // Call handlers.
    blackboard.ProcessQueuedEvents();

    // Make sure the handlers have been successfully called.
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;
    MouseClickRightHandlerCalled = false;

    // Make sure the self-removing handler has been successfully removed.
    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    REQUIRE(!MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
}

TEST_CASE("ClearEventHandlers", "[BlackboardTest]") {
    Blackboard blackboard;

    // Register event handlers.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseClickMiddleHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::No);

    // Clear event handlers for left and right mouse click.
    blackboard.ClearEventHandlers(eventMouseClickLeft);
    blackboard.ClearEventHandlers(eventMouseClickRight);

    // Create dummy event content.
    Object dummyObject{};

    // Post events and verify that only the active handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(!MouseClickLeftHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickMiddle, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(!MouseClickRightHandlerCalled);
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
}

TEST_CASE("PostEvent", "[BlackboardTest]") {
    Blackboard blackboard;

    // Register event handlers.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseClickMiddleHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::No);

    // Create string value.
    Value stringValue{"Thirteen"s};

    // Create numeric value.
    Value numberValue{13.0};

    // Construct event content.
    Object dummyObject{};
    dummyObject.AddValue(stringValue, numberValue);
    dummyObject.AddValue(numberValue, stringValue);

    // Post events and verify that the corresponding handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickLeftEventContent = nullptr;

    blackboard.PostEvent(eventMouseClickMiddle, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    REQUIRE(MouseClickMiddleEventContent->GetValue(stringValue));
    REQUIRE(MouseClickMiddleEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickMiddleEventContent->GetValue(numberValue));
    REQUIRE(MouseClickMiddleEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;
    MouseClickMiddleEventContent = nullptr;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue));
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue));
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
    MouseClickRightEventContent = nullptr;

    // Clear event handlers for left and right mouse click.
    blackboard.ClearEventHandlers(eventMouseClickLeft);
    blackboard.ClearEventHandlers(eventMouseClickRight);

    // Register persistent event handlers for left and right mouse click.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    // Register temporary event handlers for left and right mouse click.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::Yes);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::Yes);

    // Post events and verify that the corresponding handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickLeftEventContent = nullptr;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue));
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue)->ToNumber() ==  13);
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue));
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
    MouseClickRightEventContent = nullptr;

    // Post events again and verify that only the persistent event handlers have been called.
    blackboard.PostEvent(eventMouseClickLeft, dummyObject);
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickLeftEventContent = nullptr;

    blackboard.PostEvent(eventMouseClickRight, dummyObject);
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(MouseClickRightHandlerCalled);
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue));
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue));
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
    MouseClickRightEventContent = nullptr;
}

TEST_CASE("PostAndProcessQueuedEvents", "[BlackboardTest]") {
    Blackboard blackboard;

    // Register event handlers.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseClickMiddleHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickMiddle, MouseEventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::No);

    // Create string value.
    Value stringValue{"Thirteen"s};

    // Create numeric value.
    Value numberValue{13.0};

    // Construct event content.
    Object dummyObject{};
    dummyObject.AddValue(stringValue, numberValue);
    dummyObject.AddValue(numberValue, stringValue);

    // Post queued event.
    blackboard.PostQueuedEvent(eventMouseClickLeft, dummyObject);

    // Process queued event.
    blackboard.ProcessQueuedEvents();

    // Verify that the corresponding handlers have been called.
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickLeftEventContent = nullptr;

    // Post queued events.
    blackboard.PostQueuedEvent(eventMouseClickMiddle, dummyObject);
    blackboard.PostQueuedEvent(eventMouseClickRight, dummyObject);

    // Process queued events.
    blackboard.ProcessQueuedEvents();

    // Verify that the corresponding handlers have been called.
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickMiddleHandlerCalled);
    REQUIRE(MouseClickMiddleEventContent->GetValue(stringValue));
    REQUIRE(MouseClickMiddleEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickMiddleEventContent->GetValue(numberValue));
    REQUIRE(MouseClickMiddleEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickMiddleHandlerCalled = false;
    MouseClickMiddleEventContent = nullptr;

    REQUIRE(MouseClickRightHandlerCalled);
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue));
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue));
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
    MouseClickRightEventContent = nullptr;

    // Clear event handlers for left and right mouse click.
    blackboard.ClearEventHandlers(eventMouseClickLeft);
    blackboard.ClearEventHandlers(eventMouseClickRight);

    // Register persistent event handlers for left and right mouse click.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseClickLeftHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, MouseClickRightHandler, CallEventHandlerOnce::No);

    // Register temporary event handlers for left and right mouse click.
    blackboard.AddEventHandler(eventMouseClickLeft, MouseEventHandler, CallEventHandlerOnce::Yes);
    blackboard.AddEventHandler(eventMouseClickRight, MouseEventHandler, CallEventHandlerOnce::Yes);

    // Post queued events.
    blackboard.PostQueuedEvent(eventMouseClickLeft, dummyObject);
    blackboard.PostQueuedEvent(eventMouseClickRight, dummyObject);

    // Process queued events.
    blackboard.ProcessQueuedEvents();

    // Verify that the corresponding handlers have been called.
    REQUIRE(MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickLeftEventContent = nullptr;

    REQUIRE(MouseClickRightHandlerCalled);
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue));
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue));
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
    MouseClickRightEventContent = nullptr;

    // Post events again.
    blackboard.PostQueuedEvent(eventMouseClickLeft, dummyObject);
    blackboard.PostQueuedEvent(eventMouseClickRight, dummyObject);

    // Process queued events.
    blackboard.ProcessQueuedEvents();

    // Verify that only the persistent event handlers have been called.
    REQUIRE(!MouseEventHandlerCalled);
    REQUIRE(MouseClickLeftHandlerCalled);
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue));
    REQUIRE(MouseClickLeftEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickLeftHandlerCalled = false;
    MouseClickLeftEventContent = nullptr;

    REQUIRE(MouseClickRightHandlerCalled);
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue));
    REQUIRE(MouseClickRightEventContent->GetValue(stringValue)->ToNumber() == 13);
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue));
    REQUIRE(MouseClickRightEventContent->GetValue(numberValue)->ToString() == "Thirteen");
    MouseEventHandlerCalled = false;
    MouseClickRightHandlerCalled = false;
    MouseClickRightEventContent = nullptr;
}

TEST_CASE("InvocationLoopStop", "[BlackboardTest]") {
    std::size_t eventHandlersCalled = 0;

    Blackboard blackboard;

    // Create event handler that stops invocation loop.
    EventHandler eventHandler = [&blackboard, &eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        blackboard.StopInvocationLoop();
        return true;
    };

    // Create dummy handlers.
    EventHandler dummyEventHandler01 = [&eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        return true;
    };
    EventHandler dummyEventHandler02 = [&eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        return true;
    };
    EventHandler dummyEventHandler03 = [&eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        return true;
    };

    // Register handlers.
    blackboard.AddEventHandler(eventMouseClickRight, dummyEventHandler01, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, dummyEventHandler02, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, eventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, dummyEventHandler03, CallEventHandlerOnce::No);

    // Create dummy event content.
    Object dummyObject{};

    // Call handlers.
    blackboard.PostEvent(eventMouseClickRight, dummyObject);

    // Make sure the invocation loop has been stopped.
    REQUIRE(eventHandlersCalled == 3);
}

TEST_CASE("InvocationLoopStopQueued", "[BlackboardTest]") {
    std::size_t eventHandlersCalled = 0;

    Blackboard blackboard;

    // Create event handler that stops invocation loop.
    EventHandler eventHandler = [&blackboard, &eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        blackboard.StopInvocationLoop();
        return true;
    };

    // Create dummy handlers.
    EventHandler dummyEventHandler01 = [&eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        return true;
    };
    EventHandler dummyEventHandler02 = [&eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        return true;
    };
    EventHandler dummyEventHandler03 = [&eventHandlersCalled](EventID, const Object&) {
        eventHandlersCalled++;
        return true;
    };

    // Register handlers.
    blackboard.AddEventHandler(eventMouseClickRight, dummyEventHandler01, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, eventHandler, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, dummyEventHandler02, CallEventHandlerOnce::No);
    blackboard.AddEventHandler(eventMouseClickRight, dummyEventHandler03, CallEventHandlerOnce::No);

    // Create dummy event content.
    Object dummyObject{};

    // Post queued event.
    blackboard.PostQueuedEvent(eventMouseClickRight, dummyObject);

    // Call handlers.
    blackboard.ProcessQueuedEvents();

    // Make sure the invocation loop has been stopped.
    REQUIRE(eventHandlersCalled == 2);
}

TEST_CASE("PostEventRequiringHandler", "[BlackboardTest]") {
    Blackboard blackboard;

    // Create string value.
    Value stringValue{"Thirteen"s};

    // Create numeric value.
    Value numberValue{13.0};

    // Construct event content.
    Object dummyObject{};
    dummyObject.AddValue(stringValue, numberValue);
    dummyObject.AddValue(numberValue, stringValue);

    // Post unhandled event.
    bool unhandledEventExceptionThrown = false;
    try {
        blackboard.PostEventRequiringHandler(eventMouseClickLeft, dummyObject);
    } catch(UnhandledEventException& unhandledEventException) {
        unhandledEventExceptionThrown = true;
        REQUIRE(unhandledEventException.event == eventMouseClickLeft);
        REQUIRE(unhandledEventException.eventContent == dummyObject);
    }
    REQUIRE(unhandledEventExceptionThrown);
}

TEST_CASE("PostQueuedEventRequiringHandler", "[BlackboardTest]") {
    Blackboard blackboard;

    // Create string value.
    Value stringValue{"Thirteen"s};

    // Create numeric value.
    Value numberValue{13.0};

    // Construct event content.
    Object dummyObject{};
    dummyObject.AddValue(stringValue, numberValue);
    dummyObject.AddValue(numberValue, stringValue);

    // Post unhandled queued event.
    blackboard.PostQueuedEventRequiringHandler(eventMouseClickLeft, dummyObject);

    // Process unhandled queued event.
    bool unhandledEventExceptionThrown = false;
    try {
        blackboard.ProcessQueuedEvents();
    } catch(UnhandledEventException& unhandledEventException) {
        unhandledEventExceptionThrown = true;
        REQUIRE(unhandledEventException.event == eventMouseClickLeft);
        REQUIRE(unhandledEventException.eventContent == dummyObject);
    }
    REQUIRE(unhandledEventExceptionThrown);
}

TEST_CASE("PostException", "[BlackboardTest]") {
    Blackboard blackboard;

    // Create event that causes an exception to be thrown.
    const Event eventCausingException = "Event causing exception";

    // Create event handler that throws a BlackboardException.
    EventHandler eventHandler = [](EventID event, const Object& eventContent) -> bool {
        throw BlackboardException(event, eventContent);
    };

    // Register event handler.
    blackboard.AddEventHandler(eventCausingException, eventHandler, CallEventHandlerOnce::No);

    // Create string value.
    Value stringValue{"Thirteen"s};

    // Create numeric value.
    Value numberValue{13.0};

    // Construct event content.
    Object dummyObject{};
    dummyObject.AddValue(stringValue, numberValue);
    dummyObject.AddValue(numberValue, stringValue);

    // Post event causing exception.
    bool blackboardExceptionThrown = false;
    try {
        blackboard.PostEvent(eventCausingException, dummyObject);
    } catch(BlackboardException& blackboardException) {
        blackboardExceptionThrown = true;
        REQUIRE(blackboardException.event == eventCausingException);
        REQUIRE(blackboardException.eventContent == dummyObject);
    }
    REQUIRE(blackboardExceptionThrown);
}

TEST_CASE("PostQueuedException", "[BlackboardTest]") {
    Blackboard blackboard;

    // Create event that causes an exception to be thrown.
    const Event queuedEventCausingException = "Event causing exception";

    // Create event handler that throws a BlackboardException.
    EventHandler queuedEventHandler = [](EventID event, const Object& eventContent) -> bool {
        throw BlackboardQueuedException(event, eventContent);
    };

    // Register event handler.
    blackboard.AddEventHandler(queuedEventCausingException, queuedEventHandler,
                               CallEventHandlerOnce::No);

    // Create string value.
    Value stringValue{"Thirteen"s};

    // Create numeric value.
    Value numberValue{13.0};

    // Construct event content.
    Object dummyObject{};
    dummyObject.AddValue(stringValue, numberValue);
    dummyObject.AddValue(numberValue, stringValue);

    // Post queued event causing exception.
    blackboard.PostQueuedEvent(queuedEventCausingException, dummyObject);

    // Process queued event.
    bool blackboardQueuedExceptionThrown = false;
    try {
        blackboard.ProcessQueuedEvents();
    } catch(BlackboardQueuedException& blackboardQueuedException) {
        blackboardQueuedExceptionThrown = true;
        REQUIRE(blackboardQueuedException.event == queuedEventCausingException);
        REQUIRE(blackboardQueuedException.eventContent == dummyObject);
    }
    REQUIRE(blackboardQueuedExceptionThrown);
}
