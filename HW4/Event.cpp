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
    return std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(playerId) + " " + std::to_string(key) + " " + std::to_string(frameDelta);
}

GravityEvent::GravityEvent(int64_t ts, Priority p, int tId) : Event(ts, p), thisId{tId} {
    eventType = GRAVITY;
}

std::string GravityEvent::toString(){
    return std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(thisId);
}

CollisionEvent::CollisionEvent(int64_t ts, Priority p, int pId, int oId) : Event(ts, p), playerId{pId}, otherId{oId} {
    eventType = COLLISION_EVENT;
}

std::string CollisionEvent::toString(){
    return std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(playerId) + " " + std::to_string(otherId);
}

SpawnEvent::SpawnEvent(int64_t ts, Priority p, int tId, SpawnPoint * sp) : Event(ts, p), thisId{tId}, spawnPoint{sp} {
    eventType = SPAWN_EVENT;
}

std::string SpawnEvent::toString(){
    return std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(thisId);
}
