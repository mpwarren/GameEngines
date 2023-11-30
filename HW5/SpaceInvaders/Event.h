#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <string>

enum EventType{
    MOVEMENT_EV,
    SHOOT_EV,
    PLAYER_DEATH_EV,
    ENEMY_DEATH_EV,
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

class MovementEvent : public Event{
    public:
        MovementEvent(int64_t ts, Priority p, char k, int64_t fd);
        std::string toString() override;
        char key;
        int64_t frameDelta;
};

#endif