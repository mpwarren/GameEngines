#include "SpawnPoint.h"

SpawnPoint::SpawnPoint(sf::Vector2f spawn, sf::Vector2f startPoint){
    spawnPoint = spawn;
    startingPoint = startPoint;
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

void SpawnPoint::translate(std::string dir, int64_t frameDelta){
    float velocity = 0.5;
    float distMoved = velocity * frameDelta;
    if(dir == TRANSFORM_LEFT){
        spawnPoint.x = spawnPoint.x - distMoved;
    }
    else{
        spawnPoint.x = spawnPoint.x + distMoved;
    }
}

void SpawnPoint::reset(){
    spawnPoint = startingPoint;
}