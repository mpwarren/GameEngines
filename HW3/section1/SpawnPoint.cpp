#include "SpawnPoint.h"

SpawnPoint::SpawnPoint(sf::Vector2f spawn){
    spawnPoint = spawn;
    objId = "SP";
}

sf::Vector2f SpawnPoint::getSpawnPoint(){
    return spawnPoint;
}

std::string SpawnPoint::getObjId(){
    return objId;
}

std::string SpawnPoint::toString(){
    return objId + " " + std::to_string(spawnPoint.x) + " " + std::to_string(spawnPoint.y);
}