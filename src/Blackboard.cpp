// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/Blackboard.h"

#include <cassert>

namespace blackboard {

using UnhandledEventException = Blackboard::UnhandledEventException;
using BlackboardException = Blackboard::BlackboardException;
using BlackboardQueuedException = Blackboard::BlackboardQueuedException;

template <typename Titerator>
constexpr Titerator nextIterator(Titerator& i) {
    auto j = i;
    return ++j;
}

//--------------------------------------------------------------------------------------------------

Blackboard::Blackboard() : owner(GetThisThreadId()),
                           currentQueuedEvents(&queuedEventsFirst),
                           nextQueuedEvents(&queuedEventsSecond),
                           processingQueuedEvents(false),
                           eventsUnderProcessingSemaphore(0),
                           currentlyInvokedHandlerId(0),
                           currentlyInvokedHandlerAutoRemoved(false),
                           currentlyInvokedHandlerRemovedItself(false) {}

Blackboard::~Blackboard() = default;

//--------------------------------------------------------------------------------------------------

EventHandlerUniqueId
Blackboard::CalculateEventHandlerId(const Blackboard::EventHandler& eventHandler) const {
    return std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(&eventHandler));
}

std::thread::id Blackboard::GetThisThreadId() const {
    return std::this_thread::get_id();
}

void Blackboard::IncrementEventsUnderProcessingSemaphore() {
    const std::lock_guard<std::mutex> lock(eventsUnderProcessingSemaphoreMutex);
    assert(eventsUnderProcessingSemaphore >= 0);
    ++eventsUnderProcessingSemaphore;
    assert(eventsUnderProcessingSemaphore >= 0);
}

void Blackboard::DecrementEventsUnderProcessingSemaphore() {
    const std::lock_guard<std::mutex> lock(eventsUnderProcessingSemaphoreMutex);
    assert(eventsUnderProcessingSemaphore >= 0);
    --eventsUnderProcessingSemaphore;
    assert(eventsUnderProcessingSemaphore >= 0);
}

EventHandlerUniqueId Blackboard::CreateEvent(EventID eventId, const EventHandler& eventHandler,
                                             CallEventHandlerOnce callOnce) {
    const auto& [iterator, success] = events.emplace(std::piecewise_construct,
                                                     std::forward_as_tuple(eventId),
                                                     std::forward_as_tuple());
    if (!success) {
        throw std::bad_alloc();
    }

    auto& [event, eventContainer] = *iterator;
    eventContainer.eventHandlerList = std::make_unique<EventHandlerList>();
    eventContainer.eventHandlerList->push_back(EventHandlerContainer(eventHandler, callOnce));

    auto& addedEventHandlerContainer = eventContainer.eventHandlerList->back();
    addedEventHandlerContainer.eventHandlerId =
            CalculateEventHandlerId(addedEventHandlerContainer.eventHandler);

    return addedEventHandlerContainer.eventHandlerId;
}

bool Blackboard::TryToRemoveEvent(Events::iterator& eventPair) {
    const std::lock_guard<std::mutex> lock(eventsUnderProcessingSemaphoreMutex);
    if (eventsUnderProcessingSemaphore == 0) {
        events.erase(eventPair);
        return true;
    }
    return false;
}

void Blackboard::CheckIfEventNeedsRemoval(Events::iterator& eventPair) {
    auto& [_, eventContainer] = *eventPair;
    if (eventContainer.deleted) {
        TryToRemoveEvent(eventPair);
    } else if (eventContainer.eventHandlerList->empty()) {
        eventContainer.deleted = true;
        TryToRemoveEvent(eventPair);
    }
}

void Blackboard::CheckIfHandlerNeedsRemoval(EventHandlerList& eventHandlerList,
                                            EventHandlerList::iterator* currentEventHandler) {
    if ((*currentEventHandler)->callOnce) {
        currentlyInvokedHandlerAutoRemoved = true;
        *currentEventHandler = eventHandlerList.erase(*currentEventHandler);
        return;
    }
    if (currentlyInvokedHandlerRemovedItself) {
        *currentEventHandler = eventHandlerList.erase(*currentEventHandler);
        currentlyInvokedHandlerRemovedItself = false;
        return;
    }
    *currentEventHandler = nextIterator(*currentEventHandler);
}

//--------------------------------------------------------------------------------------------------

EventHandlerUniqueId Blackboard::AddEventHandler(EventID eventId, const EventHandler& eventHandler,
                                                 CallEventHandlerOnce callOnce) {
    assert(GetThisThreadId() == owner);

    if (auto eventPair = events.find(eventId); eventPair != events.end()) {
        auto& [_, eventContainer] = *eventPair;
        if (eventContainer.deleted) {
            if (TryToRemoveEvent(eventPair)) {
                return CreateEvent(eventId, eventHandler, callOnce);
            }
            return 0;
        }
        eventContainer.eventHandlerList->push_back(EventHandlerContainer(eventHandler, callOnce));

        auto& addedEventHandlerContainer = eventContainer.eventHandlerList->back();
        addedEventHandlerContainer.eventHandlerId =
                CalculateEventHandlerId(addedEventHandlerContainer.eventHandler);

        return addedEventHandlerContainer.eventHandlerId;
    }

    return CreateEvent(eventId, eventHandler, callOnce);
}

void Blackboard::RemoveEventHandler(EventID eventId, EventHandlerUniqueId eventHandlerId) {
    assert(GetThisThreadId() == owner);

    auto eventPair = events.find(eventId);
    if (eventPair == events.end()) {
        assert(currentlyInvokedHandlerId == eventHandlerId && currentlyInvokedHandlerAutoRemoved);
        return;
    }

    auto& [_, eventContainer] = *eventPair;
    if (eventContainer.deleted) {
        TryToRemoveEvent(eventPair);
        return;
    }

    const auto& currentEventHandlerList = eventContainer.eventHandlerList;
    for (auto eventHandler = currentEventHandlerList->begin();
            eventHandler != currentEventHandlerList->end(); ++eventHandler) {
        if (eventHandler->eventHandlerId == eventHandlerId) {
            if (eventHandlerId == currentlyInvokedHandlerId) {
                currentlyInvokedHandlerRemovedItself = true;
            } else {
                currentEventHandlerList->erase(eventHandler);
            }

            CheckIfEventNeedsRemoval(eventPair);
            return;
        }
    }
}

void Blackboard::ClearEventHandlers(EventID eventId) {
    assert(GetThisThreadId() == owner);

    auto eventPair = events.find(eventId);
    if (eventPair == events.end()) {
        return;
    }

    auto& [_, eventContainer] = *eventPair;
    if (eventContainer.deleted) {
        TryToRemoveEvent(eventPair);
        return;
    }

    eventContainer.deleted = true;
    TryToRemoveEvent(eventPair);
}

void Blackboard::ProcessEvent(Events::iterator eventPair, const Object& eventContent) {
    auto& [event, eventContainer] = *eventPair;
    eventContainer.threadIdPostedBy = GetThisThreadId();

    const auto& currentEventHandlerList = eventContainer.eventHandlerList;
    auto currentEventHandler = currentEventHandlerList->begin();

    while (currentEventHandler != currentEventHandlerList->end() && !eventContainer.deleted) {
        currentlyInvokedHandlerId = currentEventHandler->eventHandlerId;
        auto currentEventHandlerFunction = currentEventHandler->eventHandler; // intentionally copied

        try {
            if (!currentEventHandlerFunction(event, eventContent)) {
                CheckIfHandlerNeedsRemoval(*currentEventHandlerList, &currentEventHandler);
                break;
            }
        } catch (const StopInvocationLoopException&) {
            CheckIfHandlerNeedsRemoval(*currentEventHandlerList, &currentEventHandler);
            break;
        }

        CheckIfHandlerNeedsRemoval(*currentEventHandlerList, &currentEventHandler);
    }

    currentlyInvokedHandlerId = 0;
    eventContainer.threadIdPostedBy = std::thread::id();

    CheckIfEventNeedsRemoval(eventPair);

    eventContainer.eventCondition.notify_one();
}

void Blackboard::PostEventInternal(EventID eventId, const Object& eventContent, bool requiresHandler) {
    IncrementEventsUnderProcessingSemaphore();

    auto eventPair = events.find(eventId);
    if (eventPair == events.end()) {
        DecrementEventsUnderProcessingSemaphore();
        if (requiresHandler) {
            throw UnhandledEventException(eventId, eventContent);
        }
        return;
    }

    auto& [_, eventContainer] = *eventPair;
    if (eventContainer.deleted) {
        DecrementEventsUnderProcessingSemaphore();
        if (requiresHandler) {
            auto unhandledEventException = UnhandledEventException(eventId, eventContent);
            TryToRemoveEvent(eventPair);
            throw unhandledEventException;
        }
        TryToRemoveEvent(eventPair);
        return;
    }

    if (GetThisThreadId() == eventContainer.threadIdPostedBy) {
        ProcessEvent(eventPair, eventContent);
    } else {
        std::unique_lock<std::mutex> eventMutexLock(eventContainer.eventMutex);
        eventContainer.eventCondition.wait(eventMutexLock,
                                           [&eventContainer = eventContainer] {
            return eventContainer.threadIdPostedBy == std::thread::id();
        });
        ProcessEvent(eventPair, eventContent);
    }

    DecrementEventsUnderProcessingSemaphore();
}

void Blackboard::PostEvent(EventID eventId, const Object& eventContent) {
    PostEventInternal(eventId, eventContent, false);
}

void Blackboard::PostEventRequiringHandler(EventID eventId, const Object& eventContent) {
    PostEventInternal(eventId, eventContent, true);
}

void Blackboard::PostException(EventID eventId, const Object& eventContent) {
    throw BlackboardException(eventId, eventContent);
}

void Blackboard::PostQueuedEventInternal(EventID eventId,
                                         const Object& eventContent,
                                         QueuedEvent::RequiresHandler requiresHandler,
                                         QueuedEvent::IsException isException) {
    const std::lock_guard<std::mutex> lock(processingQueuedEventsMutex);
    if (processingQueuedEvents) {
        nextQueuedEvents->emplace(eventId, eventContent, requiresHandler, isException);
    } else {
        currentQueuedEvents->emplace(eventId, eventContent, requiresHandler, isException);
    }
}

void Blackboard::PostQueuedEvent(EventID eventId, const Object& eventContent) {
    PostQueuedEventInternal(eventId, eventContent, QueuedEvent::RequiresHandler::No,
                            QueuedEvent::IsException::No);
}

void Blackboard::PostQueuedEventRequiringHandler(EventID eventId, const Object& eventContent) {
    PostQueuedEventInternal(eventId, eventContent, QueuedEvent::RequiresHandler::Yes,
                            QueuedEvent::IsException::No);
}

void Blackboard::PostQueuedException(EventID eventId, const Object& eventContent) {
    PostQueuedEventInternal(eventId, eventContent, QueuedEvent::RequiresHandler::No,
                            QueuedEvent::IsException::Yes);
}

void Blackboard::ProcessQueuedEvents() {
    IncrementEventsUnderProcessingSemaphore();

    if (GetThisThreadId() != threadIdProcessingQueuedEvents) {
        std::unique_lock<std::mutex> processingQueuedEventsMutexLock(processingQueuedEventsMutex);
        processingQueuedEventsCondition.wait(processingQueuedEventsMutexLock, [this] {
            return threadIdProcessingQueuedEvents == std::thread::id();
        });

        threadIdProcessingQueuedEvents = GetThisThreadId();
        processingQueuedEvents = true;
    }

    while (!currentQueuedEvents->empty()) {
        auto& queuedEvent = currentQueuedEvents->front();
        if (queuedEvent.isException) {
            auto blackboardException = BlackboardException(queuedEvent.event,
                                                           queuedEvent.eventContent);
            currentQueuedEvents->pop();
            throw blackboardException;
        }

        auto eventPair = events.find(queuedEvent.event);
        if (eventPair == events.end() || eventPair->second.deleted) {
            if (queuedEvent.requiresHandler) {
                auto unhandledEventException = UnhandledEventException(queuedEvent.event,
                                                                       queuedEvent.eventContent);
                currentQueuedEvents->pop();
                throw unhandledEventException;
            }
            currentQueuedEvents->pop();
            continue;
        }

        ProcessEvent(eventPair, queuedEvent.eventContent);
        currentQueuedEvents->pop();
    }

    processingQueuedEventsMutex.lock();
    processingQueuedEvents = false;
    threadIdProcessingQueuedEvents = std::thread::id();
    std::swap(currentQueuedEvents, nextQueuedEvents);
    processingQueuedEventsMutex.unlock();

    processingQueuedEventsCondition.notify_one();
    DecrementEventsUnderProcessingSemaphore();
}

void Blackboard::StopInvocationLoop() {
    throw StopInvocationLoopException();
}

//--------------------------------------------------------------------------------------------------

UnhandledEventException::UnhandledEventException(EventID event, const Object& eventContent) :
    event(event),
    eventContent(eventContent),
    description(std::string("Unhandled event exception caused while processing event '" +
                            std::string(event) + "'")) {}

const char* UnhandledEventException::what() const noexcept {
    return description.c_str();
}

//--------------------------------------------------------------------------------------------------

BlackboardException::BlackboardException(EventID event, const Object& eventContent)
    : event(event), eventContent(eventContent),
      description(std::string("Blackboard exception caused while processing event '" +
                              std::string(event) + "'")) {}

const char* BlackboardException::what() const noexcept {
    return description.c_str();
}

//--------------------------------------------------------------------------------------------------

BlackboardQueuedException::BlackboardQueuedException(EventID event, const Object& eventContent)
    : event(event), eventContent(eventContent),
      description(std::string("Blackboard exception caused while processing event '" +
                              std::string(event) + "'")) {}

const char* BlackboardQueuedException::what() const noexcept {
    return description.c_str();
}

//--------------------------------------------------------------------------------------------------

Blackboard::QueuedEvent::QueuedEvent(EventID event, const Object& eventContent,
                                     RequiresHandler requiresHandler, IsException isException)
    : event(event), eventContent(eventContent),
      requiresHandler(requiresHandler == RequiresHandler::Yes),
      isException(isException == IsException::Yes) {}

Blackboard::QueuedEvent::~QueuedEvent() = default;

//--------------------------------------------------------------------------------------------------

Blackboard::EventHandlerContainer::EventHandlerContainer(const EventHandler& eventHandler,
                                                         CallEventHandlerOnce callOnce)
    : callOnce(callOnce == CallEventHandlerOnce::Yes), eventHandlerId(0),
      eventHandler(eventHandler) {}

Blackboard::EventHandlerContainer::~EventHandlerContainer() = default;

//--------------------------------------------------------------------------------------------------

Blackboard::EventContainer::EventContainer() : eventHandlerList(), deleted(false) {}

Blackboard::EventContainer::~EventContainer() = default;

} // namespace blackboard
