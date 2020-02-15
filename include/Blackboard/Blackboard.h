// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#pragma once

#include "Blackboard/Object.h"
#include "Blackboard/Utilities.h"

#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <string_view>
#include <thread>

namespace blackboard {

using EventHandlerUniqueId = std::size_t;

class Blackboard {

public:
    using Event = std::string;
    using EventID = std::string_view;
    using EventHandler = std::function<bool(EventID, const Object&)>;

    enum class CallEventHandlerOnce : bool {
        No,
        Yes
    };

    //----------------------------------------------------------------------------------------------

    Blackboard();
    ~Blackboard();

    Blackboard(Blackboard&) = delete;
    Blackboard& operator=(const Blackboard&) = delete;

    EventHandlerUniqueId AddEventHandler(EventID eventId, const EventHandler& eventHandler,
                                         CallEventHandlerOnce callOnce);
    void RemoveEventHandler(EventID eventId, EventHandlerUniqueId eventHandlerId);
    void ClearEventHandlers(EventID eventId);

    void PostEvent(EventID eventId, const Object& eventContent);
    void PostEventRequiringHandler(EventID eventId, const Object& eventContent);
    void PostException(EventID eventId, const Object& eventContent);

    void PostQueuedEvent(EventID eventId, const Object& eventContent);
    void PostQueuedEventRequiringHandler(EventID eventId, const Object& eventContent);
    void PostQueuedException(EventID eventId, const Object& eventContent);
    void ProcessQueuedEvents();

    void StopInvocationLoop();

    //----------------------------------------------------------------------------------------------

    struct StopInvocationLoopException {};

    struct UnhandledEventException : public std::exception {
        UnhandledEventException(EventID event, const Object& eventContent);
        const char* what() const noexcept override;

        const Event event;
        const Object& eventContent;
        const std::string description;
    };

    struct BlackboardException : public std::exception {
        BlackboardException(EventID event, const Object& eventContent);
        const char* what() const noexcept override;

        const Event event;
        const Object& eventContent;
        const std::string description;
    };

    struct BlackboardQueuedException : public std::exception {
        BlackboardQueuedException(EventID event, const Object& eventContent);
        const char* what() const noexcept override;

        const Event event;
        const Object& eventContent;
        const std::string description;
    };

    //----------------------------------------------------------------------------------------------

private:
    struct QueuedEvent {
        enum class RequiresHandler : bool {
            No,
            Yes
        };

        enum class IsException : bool {
            No,
            Yes
        };

        QueuedEvent(EventID event, const Object& eventContent, RequiresHandler requiresHandler,
                    IsException isException);
        ~QueuedEvent();

        QueuedEvent(const QueuedEvent& from) = delete;
        QueuedEvent& operator=(const QueuedEvent& from) = delete;

        Event event;
        const Object& eventContent;
        bool requiresHandler;
        bool isException;
    };

    //----------------------------------------------------------------------------------------------

    struct EventHandlerContainer {
        EventHandlerContainer(const EventHandler& eventHandler, CallEventHandlerOnce callOnce);
        ~EventHandlerContainer();

        bool callOnce;
        EventHandlerUniqueId eventHandlerId;
        EventHandler eventHandler;
    };

    using EventHandlerList = std::list<EventHandlerContainer>;

    //----------------------------------------------------------------------------------------------

    struct EventContainer {
        EventContainer();
        ~EventContainer();

        EventContainer(const EventContainer& from) = delete;
        EventContainer& operator=(const EventContainer& from) = delete;

        std::unique_ptr<EventHandlerList> eventHandlerList;
        std::thread::id threadIdPostedBy;

        std::mutex eventMutex;
        std::condition_variable eventCondition;

        bool deleted;
    };

    //----------------------------------------------------------------------------------------------

    using Events = std::map<Event, EventContainer, std::less<>>;
    using QueuedEvents = std::queue<QueuedEvent>;

    void PostEventInternal(EventID eventId, const Object& eventContent, bool requiresHandler);
    void PostQueuedEventInternal(EventID eventId,
                                 const Object& eventContent,
                                 QueuedEvent::RequiresHandler requiresHandler,
                                 QueuedEvent::IsException isException);
    void ProcessEvent(Events::iterator eventPair, const Object& eventContent);

    EventHandlerUniqueId CreateEvent(EventID eventId, const EventHandler& eventHandler,
                                     CallEventHandlerOnce callOnce);
    bool TryToRemoveEvent(Events::iterator& eventPair);
    void CheckIfEventNeedsRemoval(Events::iterator& eventPair);
    void CheckIfHandlerNeedsRemoval(EventHandlerList& eventHandlerList,
                                    EventHandlerList::iterator* currentEventHandler);

    EventHandlerUniqueId CalculateEventHandlerId(const EventHandler& eventHandler) const;
    std::thread::id GetThisThreadId() const;

    void IncrementEventsUnderProcessingSemaphore();
    void DecrementEventsUnderProcessingSemaphore();

    //----------------------------------------------------------------------------------------------

    std::thread::id owner;
    Events events;

    QueuedEvents queuedEventsFirst;
    QueuedEvents queuedEventsSecond;
    QueuedEvents* currentQueuedEvents;
    QueuedEvents* nextQueuedEvents;

    bool processingQueuedEvents;
    std::thread::id threadIdProcessingQueuedEvents;
    std::mutex processingQueuedEventsMutex;
    std::condition_variable processingQueuedEventsCondition;

    int64_t eventsUnderProcessingSemaphore;
    std::mutex eventsUnderProcessingSemaphoreMutex;

    EventHandlerUniqueId currentlyInvokedHandlerId;
    bool currentlyInvokedHandlerAutoRemoved;
    bool currentlyInvokedHandlerRemovedItself;
};

} // namespace blackboard
