#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "CollidableObject.h"
#include <v8/v8.h>

class Player : public CollidableObject{
    public:
        Player(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath);
        void movePlayer(char key, int64_t frameDelta, bool sprint);
        bool isJumping();
        void setJumping();
        void jump();
        bool gravity(int64_t frameDelta);
        void setColliding(bool isColliding);
        void setIsCollidingUnder(bool iCU);
        bool checkCollision(CollidableObject* other);
        v8::Local<v8::Object> exposeToV8(v8::Isolate *isolate, v8::Local<v8::Context> &context, std::string context_name="default");
        bool colliding;



    private:
        bool jumping;
        bool isCollidingUnder;
        float jumpHeight;
        float jumpPeak;

        static void setColor(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
		static void getColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info); // note return type

        static void setColliding(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
		static void getColliding(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info); // note return type
};
#endif