#include "EventManager.h"

bool CompareEvent::operator()(Event* e1, Event* e2){
    return e1->timeStamp < e2->timeStamp;
}

EventManager::EventManager(){
    handlers = std::map<EventType, std::vector<EventHandler*>>();
    for(int typeInt = COLLISION_EVENT; typeInt != UNKNOWN; typeInt++){
        handlers[static_cast<EventType>(typeInt)] = std::vector<EventHandler*>();
    }
}

void EventManager::addHandler(std::vector<EventType> types, EventHandler* handler){
    for(EventType t : types){
        handlers[t].push_back(handler);
    }
}

void EventManager::addToQueue(Event* e){
    eventQueue.push(e);
}