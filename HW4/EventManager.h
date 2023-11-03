#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Event.h"
#include "EventHandler.h"
#include <queue>

class CompareEvent{
    public:
        bool operator() (Event* e1, Event* e2);
};

class EventManager{
    public:
        std::map<EventType, std::vector<EventHandler*>> handlers;
        std::priority_queue<Event*, std::vector<Event*>, CompareEvent> eventQueue;
        EventManager();
        void addHandler(std::vector<EventType> types, EventHandler* handler);
        void addToQueue(Event* e);
        std::mutex mutex;
};

#endif