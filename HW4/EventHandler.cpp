#include "EventHandler.h"

std::vector<std::string> parseEventMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

EventHandler::EventHandler(std::mutex* m, std::map<int, CollidableObject*>* go) : objMutex{m}, gameObjects{go} {
    context = new zmq::context_t(1);
    serverEventSender = new zmq::socket_t(*context, zmq::socket_type::push);
    serverEventSender->connect("tcp://localhost:5560");
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
            {
                std::lock_guard<std::mutex> lock(*objMutex);
                p->movePlayer(inputEvent->key, inputEvent->frameDelta);
            }
        }

        std::string playerPosEvent = std::to_string((int)MOVE_PLAYER_EVENT) + " " + std::to_string((int)MEDIUM) + " " + std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
        zmq::message_t posMsg(playerPosEvent.length());
        memcpy(posMsg.data(), playerPosEvent.c_str(), playerPosEvent.length());
        serverEventSender->send(posMsg, zmq::send_flags::none);
    }
    else if(e->eventType == GRAVITY){
        std::shared_ptr<GravityEvent> gravityEvent = std::dynamic_pointer_cast<GravityEvent>(e);
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            Player* p = (Player*)gameObjects->at(gravityEvent->thisId);
            if(p->gravity(1)){
                std::string playerPosEvent = std::to_string((int)MOVE_PLAYER_EVENT) + " " + std::to_string((int)MEDIUM) + " " + std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
                zmq::message_t posMsg(playerPosEvent.length());
                memcpy(posMsg.data(), playerPosEvent.c_str(), playerPosEvent.length());
                serverEventSender->send(posMsg, zmq::send_flags::none);
            }
        }
    }
    else if(e->eventType == COLLISION_EVENT){
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            std::shared_ptr<CollisionEvent> collisionEvent = std::dynamic_pointer_cast<CollisionEvent>(e);
            Player* p = (Player*)gameObjects->at(collisionEvent->playerId);
            CollidableObject* co = gameObjects->at(collisionEvent->otherId);

            if(co->objId == DEATH_ZONE_ID){
                std::shared_ptr<DeathEvent> e = std::make_shared<DeathEvent>(0, LOW);
                std::string deathEventString = e->toString();
                zmq::message_t deathMsg(deathEventString.length());
                memcpy(deathMsg.data(), deathEventString.c_str(), deathEventString.length());
                serverEventSender->send(deathMsg, zmq::send_flags::none);
            }
            else{
                p->setIsCollidingUnder(p->resolveColision(co));
                std::string playerPosEvent = std::to_string((int)MOVE_PLAYER_EVENT) + " " + std::to_string((int)MEDIUM) + " " + std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
                zmq::message_t posMsg(playerPosEvent.length());
                memcpy(posMsg.data(), playerPosEvent.c_str(), playerPosEvent.length());
                serverEventSender->send(posMsg, zmq::send_flags::none);
            }
            
        }


    }
    else if(e->eventType == SPAWN_EVENT){

        std::lock_guard<std::mutex> lock(*objMutex);
        std::shared_ptr<SpawnEvent> spawnEvent = std::dynamic_pointer_cast<SpawnEvent>(e);
        Player* p = (Player*)gameObjects->at(spawnEvent->thisId);
        
        p->setPosition(spawnEvent->spawnPoint->getSpawnPoint());
    }
}


ServerWorldHandler::ServerWorldHandler(std::mutex* m, std::map<int, CollidableObject*>* go, Timeline * t) : EventHandler(m, go), gameTimeline{t}{

}

void ServerWorldHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ADD_OTHER_PLAYER){
        std::shared_ptr<AddOtherPlayerEvent> addPlayerEvent = std::dynamic_pointer_cast<AddOtherPlayerEvent>(e);
        std::vector<std::string> params = parseEventMessage(addPlayerEvent->playerString);
        Player * p = new Player(stoi(params[1]), sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8]);
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            if(gameObjects->count(p->id) != 1){
                gameObjects->insert({p->id, p});
                std::cout<< "new player added " << std::endl;
            }
        }
    }
    else if(e->eventType == MOVE_PLAYER_EVENT){
        std::shared_ptr<UpdatePlayerPositionEvent> moveCharEvent = std::dynamic_pointer_cast<UpdatePlayerPositionEvent>(e);
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            gameObjects->at(moveCharEvent->playerId)->setPosition(sf::Vector2f(moveCharEvent->xPos, moveCharEvent->yPos));
        }
    }
    else if(e->eventType == DEATH_EVENT){
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            for(auto const& obj : *gameObjects){
                obj.second->reset();
            }
        }
    }
    else if(e->eventType == TRANSLATE){
        std::shared_ptr<TranslationEvent> translationEvent = std::dynamic_pointer_cast<TranslationEvent>(e);
        std::lock_guard<std::mutex> lock(*objMutex);
        for(auto const& obj : *gameObjects){
            if(obj.first != 1 && obj.second->objId != PLAYER_ID){
                obj.second->translate(translationEvent->direction, translationEvent->frameDelta);

            }
        }
    }
    else if(e->eventType == REMOVE_PLAYER){
        std::shared_ptr<RemovePlayerEvent> removePlayerEvent = std::dynamic_pointer_cast<RemovePlayerEvent>(e);
        std::lock_guard<std::mutex> lock(*objMutex);
        Player * p = (Player*)gameObjects->at(removePlayerEvent->playerId);
        gameObjects->erase(removePlayerEvent->playerId);
        delete p;

    }
    else{
        std::cout << "ERROR: unknown event sent to world handler\n";
    }
}


ClientWorldHandler::ClientWorldHandler(std::mutex* m, std::map<int, CollidableObject*>* go, Timeline * t, int id) : EventHandler(m, go), gameTimeline{t}, clientId{id}{

}

void ClientWorldHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ADD_OTHER_PLAYER){
        std::shared_ptr<AddOtherPlayerEvent> addPlayerEvent = std::dynamic_pointer_cast<AddOtherPlayerEvent>(e);
        std::vector<std::string> params = parseEventMessage(addPlayerEvent->playerString);
        Player * p = new Player(stoi(params[1]), sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8]);
        {
            std::lock_guard<std::mutex> lock(*objMutex);
            if(gameObjects->count(p->id) != 1){
                gameObjects->insert({p->id, p});
                std::cout<< "new player added " << std::endl;
            }
        }
    }
    else if(e->eventType == DEATH_EVENT){
        std::lock_guard<std::mutex> lock(*objMutex);
        gameObjects->at(clientId)->reset();
    }
    else if(e->eventType == TRANSLATE){
        std::shared_ptr<TranslationEvent> translateEvent = std::dynamic_pointer_cast<TranslationEvent>(e);
        //if we are not the player that moved it, translate
        if(clientId != translateEvent->playerId){
            Player * p = (Player*)gameObjects->at(clientId);
            p->translate(translateEvent->direction, translateEvent->frameDelta);
            std::string playerPosEvent = std::to_string((int)MOVE_PLAYER_EVENT) + " " + std::to_string((int)MEDIUM) + " " + std::to_string(p->id) + " " + std::to_string(p->getPosition().x) + " " + std::to_string(p->getPosition().y);
            zmq::message_t posMsg(playerPosEvent.length());
            memcpy(posMsg.data(), playerPosEvent.c_str(), playerPosEvent.length());
            serverEventSender->send(posMsg, zmq::send_flags::none);
        }
    }
    else if(e->eventType == REMOVE_PLAYER){
        std::shared_ptr<RemovePlayerEvent> removePlayerEvent = std::dynamic_pointer_cast<RemovePlayerEvent>(e);
        std::lock_guard<std::mutex> lock(*objMutex);
        Player * p = (Player*)gameObjects->at(removePlayerEvent->playerId);
        gameObjects->erase(removePlayerEvent->playerId);
        delete p;

    }
}

