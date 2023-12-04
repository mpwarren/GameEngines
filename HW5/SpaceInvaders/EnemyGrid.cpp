#include "EnemyGrid.h"

Enemy::Enemy(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath, int r, int c) : CollidableObject(id, size, position, texturePath), row{r}, col{c}{
    dead = false;
}

EnemyGrid::EnemyGrid(int w, int h, int s) : width{w}, height{h}, spacing{s}{
    leftCol = 0;
    direction = "R";
    distanceMovedDown = 0;
    rightCol = w - 1;
    numAlive = w * h;
    createEnemies();
}

std::string EnemyGrid::toString(){
    return ENEMY_GRID_ID + " " + std::to_string(width) + " " + std::to_string(height) + " " + std::to_string(spacing);
}

void EnemyGrid::killEnemy(int row, int col){
    enemyGrid[row][col]->dead = true;
    numAlive--;
    if(numAlive > 0){
        if(col == rightCol){
            bool allDead = true;
            for(int i = 0; i < row; i++){
                if(!enemyGrid[i][col]->dead){
                    allDead = false;
                    break;
                }
            }
            if(allDead){
                //find the rightmost alive enemy
                for(int j = col - 1; j >= 0; j--){
                    for(int i = 0; i < height; i++){
                        if(!enemyGrid[i][j]->dead){
                            rightCol = j;
                            //std::cout << "RIGHT COL IS NOW: " << std::to_string(rightCol) << std::endl;
                            return;
                        }
                    }
                }
            }
        }
        if(col == leftCol){
            bool allDead = true;
            for(int i = 0; i < row; i++){
                if(!enemyGrid[i][col]->dead){
                    allDead = false;
                    break;
                }
            }
            if(allDead){
                //find the leftmost alive enemy
                for(int j = col + 1; j < width; j++){
                    for(int i = 0; i < height; i++){
                        if(!enemyGrid[i][j]->dead){
                            leftCol = j;
                            //std::cout << "LEFT COL IS NOW: " << std::to_string(leftCol) << std::endl;
                            return;
                        }
                    }
                }
            }
        }
    }
}


void EnemyGrid::moveEnemies(int64_t frameDelta){

    float speedModifier = 0.5;

    if(direction == "R"){
        for(std::vector<Enemy*> row : enemyGrid){
            for(Enemy* e : row){
                e->move(frameDelta * speedModifier, 0);
            }
        }

        if(enemyGrid[0][rightCol]->getPosition().x > SCENE_WIDTH - 50){
            direction = "D";
            moveRightNext = false;
        }
    }
    else if(direction == "D"){
        for(std::vector<Enemy*> row : enemyGrid){
            for(Enemy* e : row){
                e->move(0, frameDelta * speedModifier);
            }
        }
        distanceMovedDown += frameDelta;

        if(distanceMovedDown > 50){
            distanceMovedDown = 0;
            if(moveRightNext){
                direction = "R";
            }
            else{
                direction = "L";
            }
        }
    }
    else if(direction == "L"){
        for(std::vector<Enemy*> row : enemyGrid){
            for(Enemy* e : row){
                e->move(-frameDelta * speedModifier, 0);
            }
        }

        if(enemyGrid[0][leftCol]->getPosition().x < 0){
            direction = "D";
            moveRightNext = true;
        }
    }
}

void EnemyGrid::createEnemies(){
    int enemyWidth = 50;
    int startingX = 50;
    int startingY = 50;
    int currentY = startingY;
    int currentX = startingX;
    enemyGrid.clear();
    for(int i = 0; i < height; i++){
        enemyGrid.push_back(std::vector<Enemy*>());
        for(int j = 0; j < width; j++){
            enemyGrid[i].push_back(new Enemy(-1, sf::Vector2f(enemyWidth, 20), sf::Vector2f(currentX, currentY), "", i, j));
            currentX += enemyWidth + spacing;
        }
        currentX = startingX;
        currentY += startingY;
    }
}

v8::Local<v8::Object> EnemyGrid::exposeToV8(v8::Isolate *isolate, v8::Local<v8::Context> &context, std::string context_name){
    std::vector<v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>> v;
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("numAlive", getNumAlive, setNumAlive));
    return v8helpers::exposeToV8("enemies", this, v, isolate, context, context_name);
}

void EnemyGrid::setNumAlive(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
	void* ptr = wrap->Value();
	static_cast<EnemyGrid*>(ptr)->numAlive = value->Int32Value();
}

void EnemyGrid::getNumAlive(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int numAlive = static_cast<EnemyGrid*>(ptr)->numAlive;
    info.GetReturnValue().Set(numAlive);
}