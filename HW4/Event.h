#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include "GameShapes/Player.h"
#include "SpawnPoint.h"

//KEEP UNKNOWN AT THE BOTTOM, USED FOR ITERATION
enum EventType{
    COLLISION_EVENT,
    SPAWN_EVENT,
    DEATH_EVENT,
    INPUT_MOVEMENT,
    INPUT_OTHER,
    TRANSLATE_LEFT,
    TRANSLATE_RIGHT,
    ADD_OTHER_PLAYER,
    MOVE_OTHER_PLAYER,
    REMOVE_PLAYER,
    GRAVITY,
    UNKNOWN
};

enum Priority{
    HIGH,
    MEDIUM,
    LOW
};

class Event{
    public:
        Event(int64_t ts, Priority p);
        virtual std::string toString();
        EventType eventType;
        int64_t timeStamp;
        Priority priority;
};

class MovementInputEvent : public Event{
    public:
        MovementInputEvent(int64_t ts, Priority p, int id, char k, int64_t fd);
        std::string toString() override;
        int playerId;
        char key;
        int64_t frameDelta;
};

class GravityEvent : public Event{
    public:
        GravityEvent(int64_t ts, Priority p, int tId);
        std::string toString() override;
        int thisId;
};

class CollisionEvent : public Event{
    public:
        CollisionEvent(int64_t ts, Priority p, int pId, int oId);
        std::string toString() override;
        int playerId;
        int otherId;
};

class SpawnEvent : public Event{
    public:
        SpawnEvent(int64_t ts, Priority p, int tId, SpawnPoint * sp);
        std::string toString() override;
        int thisId;
        SpawnPoint * spawnPoint;
};


#endif 