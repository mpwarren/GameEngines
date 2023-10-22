#include "SideBoundry.h"

SideBoundry::SideBoundry(int id, sf::Vector2f position, std::string thisSide) : CollidableObject(id, sf::Vector2f(1, SCENE_HEIGHT), position, ""){
    side = thisSide;
}