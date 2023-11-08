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
        zmq::context_t* context;
        zmq::socket_t* serverEventSender;
};

class PlayerHandler : public EventHandler{
    public:
        PlayerHandler(std::mutex* m, std::map<int, CollidableObject*>* go);
        void onEvent(std::shared_ptr<Event> e) override;
};

class ServerWorldHandler : public EventHandler{
    public:
        Timeline * gameTimeline;
        ServerWorldHandler(std::mutex* m, std::map<int, CollidableObject*>* go, Timeline * t);
        void onEvent(std::shared_ptr<Event> e) override;
};

class ClientWorldHandler : public EventHandler{
    public:
        Timeline * gameTimeline;
        ClientWorldHandler(std::mutex* m, std::map<int, CollidableObject*>* go, Timeline * t, int id);
        void onEvent(std::shared_ptr<Event> e) override;
        int clientId;

};

#endif