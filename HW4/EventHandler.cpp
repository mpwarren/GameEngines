#include "EventHandler.h"

std::vector<std::string> parseEventMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

EventHandler::EventHandler(std::mutex* m, std::map<int, CollidableObject*>* go) : objMutex{m}, gameObjects{go} {

}

void EventHandler::onEvent(std::shared_ptr<Event> e){

}

PlayerHandler::PlayerHandler(std::mutex* m, std::map<int, CollidableObject*>* go) : EventHandler(m, go){

}

void PlayerHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == INPUT_MOVEMENT){
        //move the player
        std::shared_ptr<MovementInputEvent> inputEvent = std::dynamic_pointer_cast<MovementInputEvent>(e);
        Player* p = (Player*)gameObjects->at(inputEvent->playerId);

        if(inputEvent->key == 'W'){
            std::lock_guard<std::mutex> lock(*objMutex);
            p->setJumping();
        }
        else if(inputEvent->key == 'A' || inputEvent->key == 'D'){
            //std::string playerPosString;
            {
                std::lock_guard<std::mutex> lock(*objMutex);
                p->movePlayer(inputEvent->key, inputEvent->frameDelta);

                //playerPosString = std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
            }

            //push new player position
            //zmq::message_t posMsg(playerPosString.length());
            //memcpy(posMsg.data(), playerPosString.c_str(), playerPosString.length());
            //playerPosPub->send(posMsg, zmq::send_flags::none);
            //std::cout << "DONE WITH INPUT" << std::endl;
        }
    }
    else if(e->eventType == GRAVITY){
        //std::string playerPosString = "";
        std::shared_ptr<GravityEvent> gravityEvent = std::dynamic_pointer_cast<GravityEvent>(e);
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            Player* p = (Player*)gameObjects->at(gravityEvent->thisId);
            //playerPosString = std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
        }
        //push new player position
        //zmq::message_t posMsg(playerPosString.length());
        //memcpy(posMsg.data(), playerPosString.c_str(), playerPosString.length());
        //playerPosPub->send(posMsg, zmq::send_flags::none);
    }
    else if(e->eventType == COLLISION_EVENT){
        //std::string playerPosString;
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            std::shared_ptr<CollisionEvent> collisionEvent = std::dynamic_pointer_cast<CollisionEvent>(e);
            Player* p = (Player*)gameObjects->at(collisionEvent->playerId);
            CollidableObject* co = gameObjects->at(collisionEvent->otherId);
            
            p->setIsCollidingUnder(p->resolveColision(co));

            //playerPosString = std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
        }
        //zmq::message_t posMsg(playerPosString.length());
        //memcpy(posMsg.data(), playerPosString.c_str(), playerPosString.length());
        //playerPosPub->send(posMsg, zmq::send_flags::none);

    }
    else if(e->eventType == SPAWN_EVENT){

        std::lock_guard<std::mutex> lock(*objMutex);
        std::shared_ptr<SpawnEvent> spawnEvent = std::dynamic_pointer_cast<SpawnEvent>(e);
        Player* p = (Player*)gameObjects->at(spawnEvent->thisId);
        
        p->setPosition(spawnEvent->spawnPoint->getSpawnPoint());
    }
}


WorldHandler::WorldHandler(std::mutex* m, std::map<int, CollidableObject*>* go, Timeline * t) : EventHandler(m, go), gameTimeline{t}{

}

void WorldHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ADD_OTHER_PLAYER){
        std::cout << "HANDLING NEW PLAYER EVENT";
        std::shared_ptr<AddOtherPlayerEvent> addPlayerEvent = std::dynamic_pointer_cast<AddOtherPlayerEvent>(e);
        std::vector<std::string> params = parseEventMessage(addPlayerEvent->playerString);
        Player * p = new Player(stoi(params[1]), sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8]);
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            if(gameObjects->count(p->id) != 1){
                gameObjects->insert({p->id, p});
            }
        }
    }
}