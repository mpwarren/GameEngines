#include <zmq.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include "Timeline.h"
#include <iostream>
#include <thread>
#include <cmath>
#include "SpawnPoint.h"


std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void movePlatforms(std::map<int, CollidableObject*>* gameObjects){
    //Initalize socket
    zmq::context_t context (2);

    zmq::socket_t platformMovementSocket (context, zmq::socket_type::pub);

    //int conflate = 1;
    //platformMovementSocket.setsockopt(ZMQ_CONFLATE, &conflate, sizeof(conflate));

    platformMovementSocket.bind ("tcp://*:5555");

    //Create Timelines
    Timeline anchorTimeline;
    Timeline platformTime(&anchorTimeline);
    int64_t lastTime = platformTime.getTime();

    std::vector<MovingPlatform*> movingPlatforms;
    for(auto const& obj : *gameObjects){
        if(obj.second->objId == "MP"){
            movingPlatforms.push_back((MovingPlatform*)obj.second);
        }
    }

    while (true){

        int64_t currentTime = platformTime.getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;
        if(frameDelta != 0){
            for(MovingPlatform* obj : movingPlatforms){
                obj->movePosition(frameDelta);

                std::string platformPositionStr = "" + std::to_string(obj->id) + " " + std::to_string(obj->getPosition().x) + " " + std::to_string(obj->getPosition().y) + "\0";
                zmq::message_t posMessage(platformPositionStr.length());
                memcpy(posMessage.data(), platformPositionStr.c_str(), platformPositionStr.length());
                platformMovementSocket.send(posMessage, zmq::send_flags::none);

            }
        }

    }
}



int main(){

    //Create Timelines
    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);

    //Set Spawn Point
    SpawnPoint sp(sf::Vector2f(50, 50));

    int id = 1;

    std::map<int, CollidableObject*> gameObjects;

    Platform platform(id, sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "");
    gameObjects[platform.id] = &platform;
    id++;

    MovingPlatform horzPlatform(id, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.3, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    gameObjects[horzPlatform.id] = &horzPlatform;
    id++;

    MovingPlatform vertPlatform(id, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.3, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    gameObjects[vertPlatform.id] = &vertPlatform;
    id++;

    std::thread platformThread(movePlatforms, &gameObjects);

    //Initalize sockets
    zmq::context_t context (1);
    zmq::socket_t playerConnectionSocket (context, zmq::socket_type::rep);
    playerConnectionSocket.bind ("tcp://*:5556");

    zmq::socket_t playerPositionPublisher (context, zmq::socket_type::pub);
    playerPositionPublisher.bind ("tcp://*:5557");

    while(true){
        zmq::message_t playerMessage;
        playerConnectionSocket.recv(playerMessage, zmq::recv_flags::none);

        if(playerMessage.to_string() == "np"){
            //Send the client their player's id
            zmq::message_t newResponse(std::to_string(id).length());
            memcpy(newResponse.data(), std::to_string(id).c_str(), std::to_string(id).length());
            playerConnectionSocket.send(newResponse, zmq::send_flags::sndmore);

            //add new player to list of objects
            gameObjects[id] = new Player(id, sf::Vector2f(50, 50), sp.getSpawnPoint(), "");

            //send platform and other collidable objects
            for(int i = 1; i <= id; i++){
                zmq::message_t platformMessage(gameObjects[i]->toString().length());
                memcpy(platformMessage.data(), gameObjects[i]->toString().c_str(), gameObjects[i]->toString().length());
                playerConnectionSocket.send(platformMessage, zmq::send_flags::sndmore);
            }

            //send spawn point
            std::string spString = sp.toString();
            zmq::message_t spawnPointMessage(spString.length());
            memcpy(spawnPointMessage.data(), spString.c_str(), spString.length());
            playerConnectionSocket.send(spawnPointMessage, zmq::send_flags::none);

            //send new player out to existing clients
            std::string newPlayerString = gameObjects[id]->toString();
            zmq::message_t newPlayerMessage(newPlayerString.length());
            memcpy(newPlayerMessage.data(), newPlayerString.c_str(), newPlayerString.length());
            playerPositionPublisher.send(newPlayerMessage, zmq::send_flags::none);

            id++;
        }
        else{
            //a player moved
            
            //update the server's game object list with the new position
            std::vector<std::string> words = parseMessage(playerMessage.to_string());

            gameObjects[stoi(words[0])]->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
            playerConnectionSocket.send(zmq::message_t(), zmq::send_flags::none);

            //send out the position to all other clients
            playerPositionPublisher.send(playerMessage, zmq::send_flags::none);
        }
    }
    
}