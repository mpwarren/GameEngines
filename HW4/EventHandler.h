#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "Event.h"
#include <mutex>
#include <zmq.hpp>
#include "Timeline.h"

class EventHandler{
    public:
        std::mutex* objMutex;
        std::map<int, CollidableObject*>* gameObjects;
        EventHandler(std::mutex* m, std::map<int, CollidableObject*>* go);
        virtual void onEvent(std::shared_ptr<Event> e);
};

class PlayerHandler : public EventHandler{
    public:
        PlayerHandler(std::mutex* m, std::map<int, CollidableObject*>* go);
        void onEvent(std::shared_ptr<Event> e) override;
};

class WorldHandler : public EventHandler{
    public:
        Timeline * gameTimeline;
        WorldHandler(std::mutex* m, std::map<int, CollidableObject*>* go, Timeline * t);
        void onEvent(std::shared_ptr<Event> e) override;
};

#endif