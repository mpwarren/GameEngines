#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "CollidableObject.h"
#include "Constants.h"
#include <v8.h>
#include "ScriptManager.h"
#include <libplatform/libplatform.h>
#include "v8helpers.h"

class Player : public CollidableObject{
    public:
        Player(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath);
        void movePlayer(char key, int64_t frameDelta);
        int lives;
        int score;
        v8::Local<v8::Object> exposeToV8(v8::Isolate *isolate, v8::Local<v8::Context> &context, std::string context_name="default");
        static void setOutlineColorR(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
        static void getOutlineColorR(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
        static void setColor(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
		static void getColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info); // note return type
        static void setLives(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
		static void getLives(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info); // note return type

};

#endif