#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include "GameShapes/Player.h"

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

#endif 