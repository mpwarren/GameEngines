#ifndef GEN_H
#define GEN_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "MovingPlatform.h"
#include "CollidableObject.h"

std::vector<MovingPlatform*> createMovingPlatforms();

std::vector<Platform>* createStaticPlatforms();

#endif