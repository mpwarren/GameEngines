#ifndef ENEMY_GRID_H
#define ENEMY_GRID_H

#include "CollidableObject.h"
#include <vector>
#include <mutex>
#include "Constants.h"
#include <v8.h>
#include <libplatform/libplatform.h>
#include "v8helpers.h"

class Enemy : public CollidableObject{
    public:
        Enemy(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath, int r, int c);
        bool dead;
        int row;
        int col;
};

class EnemyGrid{
    public:
        EnemyGrid(int w, int h, int s);
        int width;
        int height;
        int spacing;    
        int leftCol;
        int rightCol; 
        int numAlive;  
        std::vector<std::vector<Enemy*>> enemyGrid;
        std::string toString();
        void killEnemy(int row, int col);
        void moveEnemies(int64_t frameDelta);
        void createEnemies();
        v8::Local<v8::Object> exposeToV8(v8::Isolate *isolate, v8::Local<v8::Context> &context, std::string context_name="default");
        static void setNumAlive(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
        static void getNumAlive(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

    private:
        std::string direction;
        float distanceMovedDown;
        bool moveRightNext;
};

#endif