#include "SpawnPoint.h"

SpawnPoint::SpawnPoint(sf::Vector2f spawn, sf::Vector2f startingPoint){
    spawnPoint = spawn;
    startingPoint = startingPoint;
    objId = SPAWN_POINT_ID;
}

sf::Vector2f SpawnPoint::getSpawnPoint(){
    return spawnPoint;
}

std::string SpawnPoint::getObjId(){
    return objId;
}

std::string SpawnPoint::toString(){
    return objId + " " + std::to_string(spawnPoint.x) + " " + std::to_string(spawnPoint.y) + " " + std::to_string(startingPoint.x) + " " + std::to_string(startingPoint.y);
}

void SpawnPoint::translate(int64_t frameDelta){
    spawnPoint.x += frameDelta;
}

void SpawnPoint::reset(){
    spawnPoint = startingPoint;
}