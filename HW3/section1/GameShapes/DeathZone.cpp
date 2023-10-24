#include "DeathZone.h"

DeathZone::DeathZone(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath) : CollidableObject(id, size, position, texturePath){
    objId = DEATH_ZONE_ID;
}