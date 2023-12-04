#include "EventManager.h"

bool CompareEvent::operator()(std::shared_ptr<Event> e1, std::shared_ptr<Event> e2){
    return e1->timeStamp > e2->timeStamp;
}

EventManager::EventManager(){
    handlers = std::map<EventType, std::vector<EventHandler*>>();
    for(int typeInt = MOVEMENT_EV; typeInt != UNKNOWN; typeInt++){
        handlers[static_cast<EventType>(typeInt)] = std::vector<EventHandler*>();
    }
}

void EventManager::addHandler(std::vector<EventType> types, EventHandler* handler){
    std::lock_guard<std::mutex> lock(mutex);
    for(EventType t : types){
        handlers[t].push_back(handler);
    }
}

void EventManager::addToQueue(std::shared_ptr<Event> e, bool shouldLock){
    {

        if(shouldLock){
            std::lock_guard<std::mutex> lock(mutex);

            if(e->priority == HIGH){
                eventQueueHigh.push(e);
            }
            else if(e->priority == MEDIUM){
                eventQueueMedium.push(e);
            }
            else if(e->priority == LOW){
                eventQueueLow.push(e);
            }
        }
        else{
            if(e->priority == HIGH){
                eventQueueHigh.push(e);
            }
            else if(e->priority == MEDIUM){
                eventQueueMedium.push(e);
            }
            else if(e->priority == LOW){
                eventQueueLow.push(e);
            }
        }

    }

}