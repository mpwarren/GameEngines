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

    zmq::socket_t heartbeatSocket(context, zmq::socket_type::pull);
    heartbeatSocket.bind("tcp://*:5561");

    zmq::socket_t eventSender (context, zmq::socket_type::push);
    eventSender.connect ("tcp://localhost:5560");

    std::map<int, int64_t> lastBeatTime;

    Timeline beatTimeline;

    while(true){
        zmq::message_t beatMessage;
        zmq::recv_result_t r = heartbeatSocket.recv(beatMessage, zmq::recv_flags::dontwait);
        if(beatMessage.to_string().length() > 0){
            int id = stoi(beatMessage.to_string());
            lastBeatTime[id] = beatTimeline.getTime();
        }

        for(auto const& obj : lastBeatTime){
            int64_t currentTime = beatTimeline.getTime();
            if(currentTime - obj.second > 2500){
                //send message to delete player on all clients
                std::string deleteStr = std::to_string(REMOVE_PLAYER) + " 0 " + std::to_string(LOW) + " " + std::to_string(obj.first);
                zmq::message_t deletePlayerMessage(deleteStr.length());
                memcpy(deletePlayerMessage.data(), deleteStr.c_str(), deleteStr.length());
                eventSender.send(deletePlayerMessage, zmq::send_flags::none);

                //erase from this map
                lastBeatTime.erase(obj.first);

                break;
            }
        }
    }
    


}


void movePlatforms(Timeline* platformTime, std::map<int, CollidableObject*>* gameObjects, SpawnPoint * sp){
        //Initalize socket
    zmq::context_t context (1);

    zmq::socket_t platformMovementSocket (context, zmq::socket_type::pub);

    platformMovementSocket.bind ("tcp://*:5555");
    int64_t lastTime = platformTime->getTime();

    std::vector<MovingPlatform*> movingPlatforms;
    for(auto const& obj : *gameObjects){
        if(obj.second->objId == MOVING_PLATFORM_ID){
            movingPlatforms.push_back((MovingPlatform*)obj.second);
        }
    }

    while(true){
        int64_t currentTime = platformTime->getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;
        if(frameDelta != 0){
            std::lock_guard<std::mutex> lock(dataMutex);

            for(auto const& obj : *gameObjects){
                
                if(obj.second->objId == MOVING_PLATFORM_ID){
                    MovingPlatform* mp = (MovingPlatform*)obj.second;
                    mp->movePosition(frameDelta);
                }
                std::string platformPositionStr = std::to_string(obj.second->id) + " " + std::to_string(obj.second->getPosition().x) + " " + std::to_string(obj.second->getPosition().y) + "\0";
                zmq::message_t posMessage(platformPositionStr.length());
                memcpy(posMessage.data(), platformPositionStr.c_str(), platformPositionStr.length());
                platformMovementSocket.send(posMessage, zmq::send_flags::none);

            }
        }
    }
}


void EventPublisher(EventManager *em, Timeline* timeline){
    zmq::context_t context(1);
    zmq::socket_t serverEventListner(context, zmq::socket_type::pull);
    serverEventListner.bind("tcp://*:5560");

    zmq::socket_t eventPublisher(context, zmq::socket_type::pub);
    eventPublisher.bind("tcp://*:5559");

    while(true){
        zmq::message_t eventMessage;
        zmq::recv_result_t r = serverEventListner.recv(eventMessage, zmq::recv_flags::none);
        //std::cout << "Event: " << eventMessage.to_string() << std::endl;

        //add it to server event manager
        std::vector<std::string> params = parseMessage(eventMessage.to_string());
        EventType t = (EventType)stoi(params[0]);
        if(t == ADD_OTHER_PLAYER){
            std::string playerString = "";
            for(int i = 3; i <= 11; i++ ){
                playerString += params[i] + " ";
            }
            //std::cout << "PLAYER STRING: " << playerString << std::flush;
            std::shared_ptr<AddOtherPlayerEvent> e = std::make_shared<AddOtherPlayerEvent>(timeline->getTime(), (Priority)stoi(params[2]), playerString);
            em->addToQueue(e);
            eventPublisher.send(eventMessage, zmq::send_flags::none);
        }
        else if(t == MOVE_PLAYER_EVENT){
            std::shared_ptr<UpdatePlayerPositionEvent> e = std::make_shared<UpdatePlayerPositionEvent>(timeline->getTime(), (Priority)stoi(params[1]), stoi(params[2]), stof(params[3]), stof(params[4]));
            em->addToQueue(e);
            //eventPublisher.send(eventMessage, zmq::send_flags::none);

        }
        else if(t == DEATH_EVENT){
            std::shared_ptr<DeathEvent> e = std::make_shared<DeathEvent>(timeline->getTime(), (Priority)stoi(params[2]));
            em->addToQueue(e);
            eventPublisher.send(eventMessage, zmq::send_flags::none);

        }
        else if(t == TRANSLATE){
            //std::cout << "ADDING TRANSLATE TO QUEUE\n";
            std::shared_ptr<TranslationEvent> e = std::make_shared<TranslationEvent>(timeline->getTime(), (Priority)stoi(params[2]), params[3][0], stoi(params[4]), stoi(params[5]));
            em->addToQueue(e);
            eventPublisher.send(eventMessage, zmq::send_flags::none);
        }
        else if(t == REMOVE_PLAYER){
            std::shared_ptr<RemovePlayerEvent> e = std::make_shared<RemovePlayerEvent>(timeline->getTime(), (Priority)stoi(params[2]),stoi(params[3]));
            em->addToQueue(e);
            eventPublisher.send(eventMessage, zmq::send_flags::none);
        }
    }
}


void newPlayerFunction(int id, std::map<int, CollidableObject*>* gameObjects, SpawnPoint* sp){
    int currentId = id;
    zmq::context_t context (1);
    zmq::socket_t playerConnectionSocket (context, zmq::socket_type::rep);
    playerConnectionSocket.bind ("tcp://*:5556");

    zmq::socket_t serverEventSender (context, zmq::socket_type::push);
    serverEventSender.connect("tcp://localhost:5560");

    while(true){
        zmq::message_t pm;
        zmq::recv_result_t r = playerConnectionSocket.recv(pm, zmq::recv_flags::none);

        //Send the client their player's id
        zmq::message_t newResponse(std::to_string(currentId).length());
        memcpy(newResponse.data(), std::to_string(currentId).c_str(), std::to_string(currentId).length());
        playerConnectionSocket.send(newResponse, zmq::send_flags::sndmore);

        std::lock_guard<std::mutex> lock(dataMutex);

        //create new player
        Player p(currentId, sf::Vector2f(50, 50), sp->getSpawnPoint(), sp->getSpawnPoint(), "");

        //send platform and other collidable objects
        for(auto const& obj : *gameObjects){
            zmq::message_t platformMessage(obj.second->toString().length());
            memcpy(platformMessage.data(), obj.second->toString().c_str(), obj.second->toString().length());
            playerConnectionSocket.send(platformMessage, zmq::send_flags::sndmore);
        }

        //send player
        std::string playerString = p.toString();
        zmq::message_t playerMessage(playerString.length());
        memcpy(playerMessage.data(), playerString.c_str(), playerString.length());
        playerConnectionSocket.send(playerMessage, zmq::send_flags::sndmore);

        //send spawn point
        std::string spString = sp->toString();
        zmq::message_t spawnPointMessage(spString.length());
        memcpy(spawnPointMessage.data(), spString.c_str(), spString.length());
        playerConnectionSocket.send(spawnPointMessage, zmq::send_flags::none);

        currentId++; 

        //send new player event out to existing clients
        std::string newPlayerEventString = std::to_string((int)ADD_OTHER_PLAYER) + " 0 " + std::to_string((int)LOW) + " " + p.toString();
        zmq::message_t newPlayerMessage(newPlayerEventString.length());
        memcpy(newPlayerMessage.data(), newPlayerEventString.c_str(), newPlayerEventString.length());
        serverEventSender.send(newPlayerMessage, zmq::send_flags::none);

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


    //create event Handlers
    EventManager *eventManager = new EventManager();
    ServerWorldHandler * worldHandler = new ServerWorldHandler(&dataMutex, &gameObjects, &gameTimeline);
    eventManager->addHandler(std::vector<EventType>{MOVE_PLAYER_EVENT, ADD_OTHER_PLAYER, DEATH_EVENT, TRANSLATE, REMOVE_PLAYER}, worldHandler);

    //start threads
    std::thread platformThread(movePlatforms, &gameTimeline, &gameObjects, &sp);
    std::thread heartbeatThread(heartbeat);
    std::thread newPlayerThread(newPlayerFunction, id, &gameObjects, &sp);
    std::thread eventPublisherThread(EventPublisher, eventManager, &gameTimeline);

    //sockets
    zmq::context_t context(1);
    zmq::socket_t pullEvents(context, zmq::socket_type::pull);
    pullEvents.bind("tcp://*:5557");

    zmq::socket_t publishPlayerPos(context, zmq::socket_type::pub);
    publishPlayerPos.bind("tcp://*:5558");


    while(true){

        {
            std::lock_guard<std::mutex> lock(eventManager->mutex);
            while(!eventManager->eventQueueHigh.empty() && eventManager->eventQueueHigh.top()->timeStamp <= gameTimeline.getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueHigh.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueHigh.pop();
            }
            while(!eventManager->eventQueueMedium.empty() && eventManager->eventQueueMedium.top()->timeStamp <= gameTimeline.getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueMedium.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueMedium.pop();
            }
            while(!eventManager->eventQueueLow.empty() && eventManager->eventQueueLow.top()->timeStamp <= gameTimeline.getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueLow.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueLow.pop();
            }
        }

    }
}