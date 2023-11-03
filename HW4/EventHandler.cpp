#include "EventHandler.h"

EventHandler::EventHandler(std::mutex* m, std::map<int, CollidableObject*>* go) : objMutex{m}, gameObjects{go} {

}

void EventHandler::onEvent(Event* e){

}

PlayerHandler::PlayerHandler(std::mutex* m, std::map<int, CollidableObject*>* go) : EventHandler(m, go){
    context = new zmq::context_t(1);
    playerPosPub = new zmq::socket_t (*context, zmq::socket_type::pub);
}

void PlayerHandler::onEvent(Event *e){
    if(e->eventType == INPUT_MOVEMENT){
        //move the player
        MovementInputEvent *inputEvent = (MovementInputEvent*)e;
        std::string playerPosString;
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            Player* p = (Player*)gameObjects->at(inputEvent->playerId);
            p->movePlayer(inputEvent->key, inputEvent->frameDelta);

            playerPosString = std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
        }

        //publish new player position
        zmq::message_t posMsg;
        memcpy(posMsg.data(), playerPosString.c_str(), playerPosString.length());
        playerPosPub->send(posMsg, zmq::send_flags::none);

    }
}