#ifndef GEN_H
#define GEN_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "MovingPlatform.h"
#include "CollidableObject.h"

class PlatformGenerator{
    public:
        PlatformGenerator();
        ~PlatformGenerator();
        std::vector<MovingPlatform*> GetMovingPlatforms();
        std::vector<Platform*> GetStaticPlatforms();

    private:
        std::vector<MovingPlatform*> movingPlatforms;
        std::vector<Platform*> staticPlatforms;
};

#endif