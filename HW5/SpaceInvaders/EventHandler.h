#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "Event.h"
#include "Timeline.h"
#include <memory>
#include "Player.h"
#include "Bullet.h"

class EventHandler{
    public:
        EventHandler();
        virtual void onEvent(std::shared_ptr<Event> e);
};

class PlayerHandler : public EventHandler{
    public:
        PlayerHandler(Player* p);
        Player* player;
        void onEvent(std::shared_ptr<Event> e) override;
};

#endif