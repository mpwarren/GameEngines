#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "Event.h"

class EventHandler{
    public:
        virtual void onEvent(Event e);

}

class CollisionHandler : public EventHandler{
    public:
        void onEvent(Event e);
}

class SpawnHandler : public EventHandler{
    public:
        void onEvent(Event e);
}

class DeathHandler : public EventHandler{
    public:
        void onEvent(Event e);
}

class InputHandler : public EventHandler{
    public:
        void onEvent(Event e);
}

#endif