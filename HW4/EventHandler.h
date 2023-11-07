#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "Event.h"
#include <mutex>
#include <zmq.hpp>

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
    
    private:
        //zmq::context_t * context;
        //zmq::socket_t * playerPosPub;
};

#endif