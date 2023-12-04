#include "Event.h"

Event::Event(int64_t ts, Priority p) : timeStamp{ts}, priority{p}{

}

std::string Event::toString(){
    return std::to_string(timeStamp);
}

//---------------------------------------------MOVEMENT EVENT-------------------------------------------------

MovementEvent::MovementEvent(int64_t ts, Priority p, char k, int64_t fd) : Event(ts, p), key{k}, frameDelta{fd}{
    eventType = MOVEMENT_EV;
}

std::string MovementEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(key) + " " + std::to_string(frameDelta);
}

//-----------------------------------------END MOVEMENT EVENT-------------------------------------------------

//---------------------------------------------ENEMY DEATH EVENT-------------------------------------------------

EnemyDeathEvent::EnemyDeathEvent(int64_t ts, Priority p, int r, int c) : Event(ts, p), row{r}, col{c}{
    eventType = ENEMY_DEATH_EV;
}

std::string EnemyDeathEvent::toString(){
    return std::to_string((int)eventType) + " " + std::to_string(timeStamp) + " " + std::to_string(priority) + " " + std::to_string(row) + " " + std::to_string(col);
}

//-----------------------------------------END ENEMY DEATH EVENT-------------------------------------------------

//---------------------------------------------GAIN LIFE EVENT-------------------------------------------------

GainLifeEvent::GainLifeEvent(int64_t ts, Priority p) : Event(ts, p){
    eventType = GAIN_LIFE_EV;
}

//-----------------------------------------END GAIN LIFE EVENT-------------------------------------------------