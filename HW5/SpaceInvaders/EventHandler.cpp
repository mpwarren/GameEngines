#include "EventHandler.h"

EventHandler::EventHandler(){
    context = new zmq::context_t(1);
    serverEventSender = new zmq::socket_t(*context, zmq::socket_type::push);
    serverEventSender->connect("tcp://localhost:5558");
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
    else if(e->eventType == ENEMY_DEATH_EV){
        player->score += 100;
    }
    else if(e->eventType == GAIN_LIFE_EV){
        player->lives++;
    }
}

EnemyHandler::EnemyHandler(EnemyGrid* eg, std::mutex * gridMutex) : enemies{eg}, enemyMutex{gridMutex}{

}

void EnemyHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ENEMY_DEATH_EV){
        
        std::shared_ptr<EnemyDeathEvent> enemyDeathEvent = std::dynamic_pointer_cast<EnemyDeathEvent>(e);
        int size = enemyDeathEvent->toString().length();
        zmq::message_t eventMsg(size);
        memcpy(eventMsg.data(), enemyDeathEvent->toString().c_str(), size);
        serverEventSender->send(eventMsg, zmq::send_flags::none);
        std::lock_guard<std::mutex> lock(*enemyMutex);
        enemies->killEnemy(enemyDeathEvent->row, enemyDeathEvent->col);
    }
}

ServerHandler::ServerHandler(EnemyGrid* eg, std::mutex * gridMutex) : enemies{eg}, enemyMutex{gridMutex}{

}

void ServerHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ENEMY_DEATH_EV){
        std::shared_ptr<EnemyDeathEvent> enemyDeathEvent = std::dynamic_pointer_cast<EnemyDeathEvent>(e);

        std::lock_guard<std::mutex> lock(*enemyMutex);
        enemies->killEnemy(enemyDeathEvent->row, enemyDeathEvent->col);
    }
}

ScriptHandler::ScriptHandler(ScriptManager * manager) : sm{manager}{

}

void ScriptHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ENEMY_DEATH_EV){
        sm->runOne("gain_life", false, "player_context");
    }
}