#ifndef GEN_H
#define GEN_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "MovingPlatform.h"
#include "CollidableObject.h"

class PlatformGenerator{
    public:
        PlatformGenerator();
        std::vector<MovingPlatform> GetMovingPlatforms();
        std::vector<Platform> GetStaticPlatforms();
        std::vector<Platform> GetAllPlatforms();

    private:
        std::vector<MovingPlatform> movingPlatforms
        std::vector<Platform> staticPlatforms;
        std::vector<MovingPlatform> allPlatforms;

}
std::vector<MovingPlatform*> createMovingPlatforms();

std::vector<Platform>* createStaticPlatforms();

#endif