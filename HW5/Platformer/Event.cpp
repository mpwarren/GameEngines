#include "Event.h"

Event::Event(int64_t ts, Priority p) : timeStamp{ts}, priority{p}{

}

std::string Event::toString(){
    return std::to_string(timeStamp);
}

MovementInputEvent::MovementInputEvent(int64_t ts, Priority p, int id, char k, int64_t fd, bool sprint) : Event(ts, p), playerId{id}, key{k}, frameDelta{fd}, sprinting{sprint} {
    eventType = INPUT_MOVEMENT;
}

std::string MovementInputEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(playerId) + " " + std::to_string(key) + " " + std::to_string(frameDelta);
}

GravityEvent::GravityEvent(int64_t ts, Priority p, int tId) : Event(ts, p), thisId{tId} {
    eventType = GRAVITY;
}

std::string GravityEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(thisId);
}

CollisionEvent::CollisionEvent(int64_t ts, Priority p, int pId, int oId) : Event(ts, p), playerId{pId}, otherId{oId} {
    eventType = COLLISION_EVENT;
}

std::string CollisionEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(playerId) + " " + std::to_string(otherId);
}

SpawnEvent::SpawnEvent(int64_t ts, Priority p, int tId, SpawnPoint * sp) : Event(ts, p), thisId{tId}, spawnPoint{sp} {
    eventType = SPAWN_EVENT;
}

std::string SpawnEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(thisId);
}

AddOtherPlayerEvent::AddOtherPlayerEvent(int64_t ts, Priority p, std::string ps) : Event(ts, p), playerString{ps} {
    eventType = ADD_OTHER_PLAYER;
}

std::string AddOtherPlayerEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + playerString;
}

UpdatePlayerPositionEvent::UpdatePlayerPositionEvent(int64_t ts, Priority p, int id, float x, float y) : Event(ts, p), playerId{id}, xPos{x}, yPos{y} {
    eventType = MOVE_PLAYER_EVENT;
}

std::string UpdatePlayerPositionEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(playerId) + " " + std::to_string(xPos)  + " " + std::to_string(yPos);
}

DeathEvent::DeathEvent(int64_t ts, Priority p) : Event(ts, p) {
    eventType = DEATH_EVENT;
}

std::string DeathEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority);
}

TranslationEvent::TranslationEvent(int64_t ts, Priority p, char dir, int pId, int64_t fd) : Event(ts, p), direction{dir}, playerId{pId}, frameDelta{fd} {
    eventType = TRANSLATE;
}

std::string TranslationEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + direction + " " + std::to_string(playerId) + " " + std::to_string(frameDelta);
}

RemovePlayerEvent::RemovePlayerEvent(int64_t ts, Priority p, int pId) : Event(ts, p), playerId{pId} {
    eventType = REMOVE_PLAYER;
}

std::string RemovePlayerEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(playerId);
}
