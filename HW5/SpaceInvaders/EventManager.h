#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Event.h"
#include "EventHandler.h"
#include <queue>

class CompareEvent{
    public:
        bool operator() (std::shared_ptr<Event> e1, std::shared_ptr<Event> e2);
};

class EventManager{
    public:
        std::map<EventType, std::vector<EventHandler*>> handlers;
        std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, CompareEvent> eventQueueHigh;
        std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, CompareEvent> eventQueueMedium;
        std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, CompareEvent> eventQueueLow;

        EventManager();
        void addHandler(std::vector<EventType> types, EventHandler* handler);
        void addToQueue(std::shared_ptr<Event> e, bool shouldLock = true);
        std::mutex mutex;
};

#endif