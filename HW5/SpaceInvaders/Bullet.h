#ifndef BULLET_H
#define BULLET_H

#include "CollidableObject.h"

class Bullet : public CollidableObject{
    public:
        Bullet(float xPosition);
        void moveBullet(int64_t frameDelta);
        int width;
        int height;
};

#endif