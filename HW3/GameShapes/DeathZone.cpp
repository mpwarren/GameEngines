#include "DeathZone.h"

DeathZone::DeathZone(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath) : CollidableObject(id, size, position, startingPosition, texturePath){
    objId = DEATH_ZONE_ID;
}