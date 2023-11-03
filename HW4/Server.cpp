#include <zmq.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include "Timeline.h"
#include <iostream>
#include <thread>
#include <cmath>
#include "SpawnPoint.h"
#include "GameShapes/DeathZone.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"

std::mutex dataMutex;

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void heartbeat(){
    //Initalize socket
    zmq::context_t context(1);

    zmq::socket_t heartbeatSocket(context, zmq::socket_type::rep);
    heartbeatSocket.bind("tcp://*:5559");

    zmq::socket_t deletePlayerSocket (context, zmq::socket_type::req);
    deletePlayerSocket.connect ("tcp://localhost:5556");

    std::map<int, int64_t> lastBeatTime;

    Timeline beatTimeline;

    while(true){
        zmq::message_t beatMessage;
        heartbeatSocket.recv(beatMessage, zmq::recv_flags::dontwait);
        if(beatMessage.to_string().length() > 0){
            int id = stoi(beatMessage.to_string());
            lastBeatTime[id] = beatTimeline.getTime();
            heartbeatSocket.send(zmq::message_t(), zmq::send_flags::none);
        }

        for(auto const& obj : lastBeatTime){
            int64_t currentTime = beatTimeline.getTime();
            if(currentTime - obj.second > 5000){
                //send message to delete player on all clients
                std::string deleteStr = DELETE_SIGN + " " + std::to_string(obj.first);
                zmq::message_t deletePlayerMessage(deleteStr.length());
                memcpy(deletePlayerMessage.data(), deleteStr.c_str(), deleteStr.length());
                deletePlayerSocket.send(deletePlayerMessage, zmq::send_flags::none);
                zmq::message_t recvMsg(0);
                deletePlayerSocket.recv(recvMsg, zmq::recv_flags::none);

                //erase from this map
                lastBeatTime.erase(obj.first);

                break;
            }
        }
    }
    


}

void movePlatforms(Timeline* platformTime, std::map<int, CollidableObject*>* gameObjects, SpawnPoint * sp){
    //Initalize socket
    zmq::context_t context (2);

    zmq::socket_t platformMovementSocket (context, zmq::socket_type::pub);
    //int conflate = 1;
    //platformMovementSocket.setsockopt(ZMQ_CONFLATE, &conflate, sizeof(conflate));
    platformMovementSocket.bind ("tcp://*:5555");

    zmq::socket_t pauseListener (context, zmq::socket_type::sub);
    pauseListener.connect ("tcp://localhost:5558");
    pauseListener.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    int64_t lastTime = platformTime->getTime();

    std::vector<MovingPlatform*> movingPlatforms;
    for(auto const& obj : *gameObjects){
        if(obj.second->objId == MOVING_PLATFORM_ID){
            movingPlatforms.push_back((MovingPlatform*)obj.second);
        }
    }

    while (true){

        zmq::message_t pauseListenerMessage;
        pauseListener.recv(pauseListenerMessage, zmq::recv_flags::dontwait);

        bool translated = false;

        if(pauseListenerMessage.to_string().length() > 0){
            std::vector<std::string> words = parseMessage(pauseListenerMessage.to_string());
            if(words[0] == PAUSING_SIGN){
                if(platformTime->isPaused()){
                    int64_t elapsedTime = platformTime->unpause();
                    lastTime = platformTime->getTime();
                }
                else{
                    platformTime->pause(lastTime);
                }    
            }
            else if(words[0] == RESET_SCENE){
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    for(auto const& obj : *gameObjects){
                        obj.second->reset();
                    }
                    sp->reset();
                }
            }
            else if(words[0] == TRANSFORM_LEFT || words[0] == TRANSFORM_RIGHT){
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    for(auto const& obj : *gameObjects){
                        if(obj.first != 1 and obj.first != stoi(words[1])){
                            obj.second->translate(words[0], stoi(words[2]));
                        }
                    }

                }
                translated = true;
            }
        }


        int64_t currentTime = platformTime->getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;
        if(!translated){
            if(frameDelta != 0){
                std::lock_guard<std::mutex> lock(dataMutex);
                for(MovingPlatform* obj : movingPlatforms){
                    obj->movePosition(frameDelta);

                    std::string platformPositionStr = std::to_string(obj->id) + " " + std::to_string(obj->getPosition().x) + " " + std::to_string(obj->getPosition().y) + "\0";
                    zmq::message_t posMessage(platformPositionStr.length());
                    memcpy(posMessage.data(), platformPositionStr.c_str(), platformPositionStr.length());
                    platformMovementSocket.send(posMessage, zmq::send_flags::none);

                }
            }
        }


    }
}


void newPlayerThread(int id, std::map<int, CollidableObject*> gameObjects, SpawnPoint* sp){
    zmq::context_t context (1);
    zmq::socket_t playerConnectionSocket (context, zmq::socket_type::rep);
    playerConnectionSocket.bind ("tcp://*:5556");

    while(true){
        zmq::message_t playerMessage;
        playerConnectionSocket.recv(playerMessage, zmq::recv_flags::none);

        //Send the client their player's id
        zmq::message_t newResponse(std::to_string(id).length());
        memcpy(newResponse.data(), std::to_string(id).c_str(), std::to_string(id).length());
        playerConnectionSocket.send(newResponse, zmq::send_flags::sndmore);

        std::lock_guard<std::mutex> lock(dataMutex);

        //add new player to list of objects
        gameObjects[id] = new Player(id, sf::Vector2f(50, 50), sp->getSpawnPoint(), sp->getSpawnPoint(), "");
        std::cout<< "new player added " << std::endl;

        //send platform and other collidable objects
        for(auto const& obj : gameObjects){
            zmq::message_t platformMessage(obj.second->toString().length());
            memcpy(platformMessage.data(), obj.second->toString().c_str(), obj.second->toString().length());
            playerConnectionSocket.send(platformMessage, zmq::send_flags::sndmore);
        }

        //send spawn point
        std::string spString = sp->toString();
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
}



int main(){



    int id = 1;

    std::map<int, CollidableObject*> gameObjects;

    int groundHeight = 15;
    Platform platform(id, sf::Vector2f(SCENE_WIDTH, groundHeight), sf::Vector2f(0,SCENE_HEIGHT - groundHeight), sf::Vector2f(0,SCENE_HEIGHT - groundHeight), "");
    gameObjects[platform.id] = &platform;
    id++;

    Platform platform1(id, sf::Vector2f(200, groundHeight), sf::Vector2f(-100, 400), sf::Vector2f(-100, 400), "");
    gameObjects[platform1.id] = &platform1;
    id++;

    Platform platform2(id, sf::Vector2f(200, groundHeight), sf::Vector2f(750, 550), sf::Vector2f(750, 550), "");
    gameObjects[platform2.id] = &platform2;
    id++;

    Platform platform3(id, sf::Vector2f(700, groundHeight), sf::Vector2f(350, 200), sf::Vector2f(350, 200), "");
    gameObjects[platform3.id] = &platform3;
    id++;

    //Set Spawn Point
    SpawnPoint sp(sf::Vector2f(375, 0), sf::Vector2f(375, 0));

    MovingPlatform horzPlatform(id, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 500), sf::Vector2f(400, 500), "", Direction::horizontal, 0.5, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    gameObjects[horzPlatform.id] = &horzPlatform;
    id++;

    MovingPlatform vertPlatform(id, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), sf::Vector2f(200, 100), "", Direction::vertical, 0.5, 400);
    gameObjects[vertPlatform.id] = &vertPlatform;
    id++;

    DeathZone spike(id, sf::Vector2f(50,20), sf::Vector2f(600, SCENE_HEIGHT - groundHeight - 20), sf::Vector2f(600, SCENE_HEIGHT - groundHeight - 20), "");
    gameObjects[spike.id] = &spike;
    id++;

    //Create Timelines
    Timeline anchorTimeline;
    Timeline gameTimeline(&anchorTimeline);

    std::thread platformThread(movePlatforms, &gameTimeline, &gameObjects, &sp);
    std::thread heartbeatThread(heartbeat);


    //Initalize sockets
    zmq::context_t context (2);

    zmq::socket_t playerPositionPublisher (context, zmq::socket_type::pub);
    playerPositionPublisher.bind ("tcp://*:5557");

    zmq::socket_t eventListner (context, zmq::socket_type::rep);
    eventListner.bind ("tcp://*:5558");

    //create event Handlers
    EventManager *eventManager = new EventManager();
    PlayerHandler * playerHandler = new PlayerHandler(&dataMutex, &gameObjects);
    eventManager->addHandler(std::vector<EventType>{INPUT_MOVEMENT}, playerHandler);

    while(true){
        zmq::message_t eventMessage;
        eventListner.recv(eventMessage, zmq::recv_flags::none);

        //make the event object
        std::vector<std::string> params = parseMessage(eventMessage.to_string());
        MovementInputEvent* ev = new MovementInputEvent(stoi(params[0]), (Priority)stoi(params[1]), stoi(params[2]), params[3][0], stoi(params[4]));
        

        while(eventManager->eventQueue.top()->timeStamp <= gameTimeline.getTime()){

        }
    }
    
}