#include "SideBoundry.h"

SideBoundry::SideBoundry(int id, sf::Vector2f position, sf::Vector2f startingPosition, std::string thisSide) : CollidableObject(id, sf::Vector2f(1, SCENE_HEIGHT + 50), position, startingPosition, ""){
    side = thisSide;
    isCollidedWith = false;
}