#include "Event.h"

Event::Event(int64_t ts, Priority p) : timeStamp{ts}, priority{p}{

}

std::string Event::toString(){
    return std::to_string(timeStamp);
}

//---------------------------------------------MOVEMENT EVENT-------------------------------------------------

MovementEvent::MovementEvent(int64_t ts, Priority p, char k, int64_t fd) : Event(ts, p), key{k}, frameDelta{fd}{
    eventType = MOVEMENT_EV;
}

std::string MovementEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(key) + " " + std::to_string(frameDelta);
}

//-----------------------------------------END MOVEMENT EVENT-------------------------------------------------
