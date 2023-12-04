#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <string>
#include <iostream>

enum EventType{
    MOVEMENT_EV,
    PLAYER_DEATH_EV,
    ENEMY_DEATH_EV,
    GAIN_LIFE_EV,
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

class GainLifeEvent : public Event{
    public:
        GainLifeEvent(int64_t ts, Priority p);
};

class EnemyDeathEvent : public Event{
    public:
        EnemyDeathEvent(int64_t ts, Priority p, int r, int c);
        std::string toString() override;
        int row;
        int col;
};

#endif