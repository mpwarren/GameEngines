#ifndef EVENT_H
#define EVENT_H

enum EventType{
    COLLISION_EVENT,
    SPAWN_EVENT,
    DEATH_EVENT,
    INPUT_EVENT
};

class Event{
    public:
        EventType eventType;
        int64_t timeStamp;
}

#endif 