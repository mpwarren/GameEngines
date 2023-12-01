#ifndef ENEMY_BULLET_H
#define ENEMY_BULLET_H

#include "CollidableObject.h"

class EnemyBullet : public CollidableObject{
    public:
        EnemyBullet(float xPosition);
        void moveBullet(int64_t frameDelta);
        int width;
        int height;
};

#endif