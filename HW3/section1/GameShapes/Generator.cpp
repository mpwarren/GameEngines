#include "Generator.h"

std::vector<MovingPlatform*> createMovingPlatforms(){
    
    std::vector<MovingPlatform*> movingObjects;

    MovingPlatform* horzPlatform = new MovingPlatform(1, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.0003, 200);
    horzPlatform->setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(horzPlatform);

    MovingPlatform* vertPlatform = new MovingPlatform(2, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.0003, 400);
    vertPlatform->setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(vertPlatform);

    return movingObjects;
}

std::vector<Platform>* createStaticPlatforms(){
    std::vector<Platform>* collidableObjects = new std::vector<Platform>();

    Platform platform(3, sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");
    collidableObjects->push_back(platform);
    return collidableObjects;
}