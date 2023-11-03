#include "Event.h"

Event::Event(int64_t ts, Priority p) : timeStamp{ts}, priority{p}{

}

std::string Event::toString(){
    return std::to_string(timeStamp);
}

MovementInputEvent::MovementInputEvent(int64_t ts, Priority p, int id, char k, int64_t fd) : Event(ts, p), playerId{id}, key{k}, frameDelta{fd} {
    eventType = INPUT_MOVEMENT;
}

std::string MovementInputEvent::toString(){
    return "";
}
