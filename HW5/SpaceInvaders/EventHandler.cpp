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
    else if(e->eventType == ENEMY_DEATH_EV){
        player->score += 100;
    }
}

EnemyHandler::EnemyHandler(EnemyGrid* eg, std::mutex * gridMutex) : enemies{eg}, enemyMutex{gridMutex}{

}

void EnemyHandler::onEvent(std::shared_ptr<Event> e){
    if(e->eventType == ENEMY_DEATH_EV){
        std::shared_ptr<EnemyDeathEvent> enemyDeathEvent = std::dynamic_pointer_cast<EnemyDeathEvent>(e);
        std::lock_guard<std::mutex> lock(*enemyMutex);
        enemies->killEnemy(enemyDeathEvent->row, enemyDeathEvent->col);
    }
}