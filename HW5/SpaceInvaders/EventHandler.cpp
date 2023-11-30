#include "EventHandler.h"

EventHandler::EventHandler(){

}

void EventHandler::onEvent(std::shared_ptr<Event> e){

}

PlayerHandler::PlayerHandler(Player* p) : player{p}{

}

void PlayerHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == MOVEMENT_EV){
        std::shared_ptr<MovementEvent> movementEvent = std::dynamic_pointer_cast<MovementEvent>(e);
        player->movePlayer(movementEvent->key, movementEvent->frameDelta);
    }
}