#include "PlatformGenerator.h"

PlatformGenerator::PlatformGenerator(){
    
    MovingPlatform* horzPlatform = new MovingPlatform(1, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.0003, 200);
    horzPlatform->setFillColor(sf::Color(150, 50, 250));
    movingPlatforms.push_back(horzPlatform);

    MovingPlatform* vertPlatform = new MovingPlatform(2, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.0003, 400);
    vertPlatform->setFillColor(sf::Color(150, 50, 250));
    movingPlatforms.push_back(vertPlatform);

    Platform* platform = new Platform(3, sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");
    staticPlatforms.push_back(platform);


}

std::vector<MovingPlatform*> PlatformGenerator::GetMovingPlatforms(){
    return movingPlatforms;
}

std::vector<Platform*> PlatformGenerator::GetStaticPlatforms(){
    return staticPlatforms;
}

PlatformGenerator::~PlatformGenerator(){
    std::cout <<"DESTRUCTION" << std::endl;
    for(MovingPlatform * plat : movingPlatforms){
        std::cout <<"DESTRUCTION" << std::endl;
        delete plat;
    }
    for(Platform * plat : staticPlatforms){
        std::cout <<"DESTRUCTION" << std::endl;
        delete plat;
    }
}