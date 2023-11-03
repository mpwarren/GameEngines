#include <map>
#include "GameShapes/CollidableObject.h"
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include "GameShapes/SideBoundry.h"
#include <zmq.hpp>
#include <mutex>
#include <thread>
#include "Timeline.h"
#include "SpawnPoint.h"
#include "GameShapes/DeathZone.h"
#include <unistd.h>


std::mutex platformMutex;

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void heartbeat(int id){
    //Connect to server
    zmq::context_t context (1);
    zmq::socket_t heartbeatSocket (context, zmq::socket_type::req);
    heartbeatSocket.connect ("tcp://localhost:5559");
    std::string idString = std::to_string(id);
    int strLen = idString.length();


    while(true){
        zmq::message_t idMessage(strLen);
        memcpy(idMessage.data(), idString.c_str(), strLen);

        heartbeatSocket.send(idMessage, zmq::send_flags::none);
        zmq::message_t recvMsg(0);
        heartbeatSocket.recv(recvMsg, zmq::recv_flags::none);

        sleep(3);
    }


}

void platformMovement(std::map<int, CollidableObject*>* gameObjects){
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t platformReciever (context, zmq::socket_type::sub);
    platformReciever.connect ("tcp://localhost:5555");
    platformReciever.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t positionUpdate;
        platformReciever.recv(positionUpdate, zmq::recv_flags::none);
        std::vector<std::string> words = parseMessage(positionUpdate.to_string());
        {
            std::lock_guard<std::mutex> lock(platformMutex);
            //std::cout << "SETTING1: " << positionUpdate << std::endl;
            gameObjects->at(stoi(words[0]))->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
        }
    }
}


void playerPositionUpdates(std::map<int, CollidableObject*>* gameObjects, std::vector<Player*>* players, int thisId){
    zmq::context_t context (1);
    zmq::socket_t recievePlayerPositionSocket (context, zmq::socket_type::sub);
    recievePlayerPositionSocket.connect ("tcp://localhost:5557");
    recievePlayerPositionSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t positionUpdate;
        recievePlayerPositionSocket.recv(positionUpdate, zmq::recv_flags::none);
        std::vector<std::string> words = parseMessage(positionUpdate.to_string());
        
        //NEW PLAYER
        if(words[0] == PLAYER_ID){
            int id = stoi(words[1]);
            Player* newPlayer = new Player(id, sf::Vector2f(stof(words[2]), stof(words[3])), sf::Vector2f(stof(words[4]), stof(words[5])), sf::Vector2f(stof(words[6]), stof(words[7])), words[8]);
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                gameObjects->insert(std::pair<int, CollidableObject*>(id, newPlayer));
                players->push_back(newPlayer);
            }
        }
        else if(words[0] == DELETE_SIGN){
            int id = stoi(words[1]);
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                std::cout << "DELETING1: " << positionUpdate << std::endl;
                delete gameObjects->at(id);
                gameObjects->erase(id);              
            }
        }
        else{
            int currentId = stoi(words[0]);
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                gameObjects->at(currentId)->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
            }
        }
    }    
}


int main(){

    //set spawnpoint to a base value for now so player can be made
    SpawnPoint sp(sf::Vector2f(0,0), sf::Vector2f(0,0));

    std::map<int, CollidableObject*> gameObjects;
    std::vector<CollidableObject*> collidableObjects;
    std::vector<DeathZone*> deathZones;
    std::vector<Player*> players;

    //Connect to server
    zmq::context_t context (3);
    zmq::socket_t newPlayerSocket (context, zmq::socket_type::req);
    newPlayerSocket.connect ("tcp://localhost:5556");

    zmq::socket_t pauseListener (context, zmq::socket_type::sub);
    pauseListener.connect ("tcp://localhost:5558");
    pauseListener.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::message_t newConnection(2);
    memcpy(newConnection.data(), NEW_PLAYER_SIGN.c_str(), 2);
    newPlayerSocket.send(newConnection, zmq::send_flags::none);

    //Get Id and create player object
    zmq::message_t idMessage(2);
    newPlayerSocket.recv(idMessage, zmq::recv_flags::none);
    std::cout << idMessage.to_string() << std::endl;
    int thisId = stoi(idMessage.to_string());
    std::cout << "Connected with ID: " << idMessage.to_string() << std::endl;

    int numPlatforms = 0;
    std::vector<sf::Color> platformColors{ sf::Color(252, 186, 3), sf::Color(53, 252, 3), sf::Color(3, 78, 252)};
    int moreToRead = 1;
    while(moreToRead != 0){
        zmq::message_t objectMessage;
        newPlayerSocket.recv(objectMessage, zmq::recv_flags::none);
        std::vector<std::string> params = parseMessage(objectMessage.to_string());
        
        if(params[0] == PLATFORM_ID){
            int id = stoi(params[1]);
            Platform* pt = new Platform(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8]);
            pt->setFillColor(platformColors[numPlatforms % platformColors.size()]);
            numPlatforms++;
            gameObjects[id] = pt;
            collidableObjects.push_back(pt);
        }
        else if(params[0] == MOVING_PLATFORM_ID){
            int id = stoi(params[1]);
            MovingPlatform* mp = new MovingPlatform(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8], (Direction)stoi(params[9]), stof(params[10]), stoi(params[11]));
            gameObjects[id] = mp;
            collidableObjects.push_back(mp);
        }
        else if(params[0] == PLAYER_ID){
            int id = stoi(params[1]);
            gameObjects[id] = new Player(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8]);
            players.push_back((Player*)gameObjects[id]);
        }
        else if(params[0] == SPAWN_POINT_ID){
            sp = SpawnPoint(sf::Vector2f(stof(params[1]), stof(params[2])), sf::Vector2f(stof(params[3]), stof(params[4])));
        }
        else if(params[0] == DEATH_ZONE_ID){
            int id = stoi(params[1]);
            DeathZone* dz = new DeathZone(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), sf::Vector2f(stof(params[6]), stof(params[7])), params[8]);
            dz->setFillColor(sf::Color(255, 0, 0));
            gameObjects[id] = dz;
            deathZones.push_back(dz);
        }
        else{
            std::cout << "ERROR: UNKNOWN OBJECT TYPE RECIEVED FROM SERVER" << std::endl;
        }

        size_t moreSize = sizeof(moreToRead);
        newPlayerSocket.getsockopt(ZMQ_RCVMORE, &moreToRead, &moreSize);
    }

    Player * thisPlayer = (Player*)gameObjects[thisId];
    thisPlayer->setFillColor(sf::Color(100, 100, 100));

    sf::RenderWindow window(sf::VideoMode(SCENE_WIDTH, SCENE_HEIGHT), "My Window", sf::Style::Default);

    //generate side boundry
    std::vector<SideBoundry*> boundaries;
    int distanceFromEdge = 50;
    SideBoundry* boundry1 = new SideBoundry(100, sf::Vector2f(SCENE_WIDTH - distanceFromEdge , -50), sf::Vector2f(SCENE_WIDTH - distanceFromEdge , -50), RIGHT_SIDE);
    boundaries.push_back(boundry1);
    SideBoundry* boundry2 = new SideBoundry(101, sf::Vector2f(distanceFromEdge, -50), sf::Vector2f(distanceFromEdge, -50), LEFT_SIDE);
    boundaries.push_back(boundry2);


    std::thread platformThread(platformMovement, &gameObjects);
    std::thread playerThread(playerPositionUpdates, &gameObjects, &players, thisId);
    std::thread heartbeatThread(heartbeat, thisId);


    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);
    int64_t lastTime = gameTime.getTime();

    while(window.isOpen()){
        
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::P){
                    zmq::message_t pauseMessage(1);
                    memcpy(pauseMessage.data(), "P", 1);
                    newPlayerSocket.send(pauseMessage, zmq::send_flags::none);
                    zmq::message_t rep(0);
                    newPlayerSocket.recv(rep, zmq::recv_flags::none); 
                }
                if(event.key.code == sf::Keyboard::Z){
                    gameTime.changeTic(TIC_HALF);
                    lastTime = gameTime.getTime();
                }
                if(event.key.code == sf::Keyboard::X){
                    gameTime.changeTic(TIC_NORMAL);
                    lastTime = gameTime.getTime();
                }
                if(event.key.code == sf::Keyboard::C){
                    gameTime.changeTic(TIC_TWO_TIMES);
                    lastTime = gameTime.getTime();
                }
            }
        }

        zmq::message_t pauseListenerMessage;
        pauseListener.recv(pauseListenerMessage, zmq::recv_flags::dontwait);
        if(pauseListenerMessage.to_string() == PAUSING_SIGN){
            if(gameTime.isPaused()){
                int64_t elapsedTime = gameTime.unpause();
                lastTime = gameTime.getTime();
            }
            else{
                gameTime.pause(lastTime);
            }    
        }
        else if(pauseListenerMessage.to_string() == RESET_SCENE){
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                for(auto const& obj : gameObjects){
                    obj.second->setPosition(obj.second->startingPoint);
                }
                sp.reset();
            }

        }
        //TRANSLATION
        else if(pauseListenerMessage.to_string().length() > 0){
            std::vector<std::string> words = parseMessage(pauseListenerMessage.to_string());
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                for(auto const& obj : gameObjects){
                    if(obj.second->objId != MOVING_PLATFORM_ID && obj.second->id != stoi(words[1]) && obj.second->id != 1){
                        obj.second->translate(words[0], stoi(words[2]));
                    }
                }
                sp.translate(words[0], stoi(words[2]));
            }

        }        

        window.clear(sf::Color::Black);


        int64_t currentTime = gameTime.getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;

        bool moved = false;
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            thisPlayer->setJumping();
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            thisPlayer->movePlayer('A', frameDelta);
            moved = true;
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            thisPlayer->movePlayer('D', frameDelta);
            moved = true;
        }

        bool died = false;
        for(DeathZone* dz : deathZones){
            if(thisPlayer->getGlobalBounds().intersects(dz->getGlobalBounds())){
                died = true;
                zmq::message_t resetMessage(2);
                memcpy(resetMessage.data(), RESET_SCENE.c_str(), 2);
                newPlayerSocket.send(resetMessage, zmq::send_flags::none);
                zmq::message_t rep(0);
                newPlayerSocket.recv(rep, zmq::recv_flags::none); 
                break;
            }
        }

        if(!died){
            //gravity
            float ground = 585;
            bool playerColliding = (thisPlayer->getPosition().y + 50 >= ground);
            thisPlayer->gravity(frameDelta, playerColliding);

            {
                std::lock_guard<std::mutex> lock(platformMutex);
                for(CollidableObject* obj : collidableObjects){
                    if(thisPlayer->getGlobalBounds().intersects(obj->getGlobalBounds())){
                        thisPlayer->resolveColision(obj);
                        playerColliding = true;
                    }
                }

                thisPlayer->setColliding(playerColliding);
            }
                
            std::string translationString;

            for(SideBoundry* sb : boundaries){
                sb->isCollidedWith = false;
                if(thisPlayer->getGlobalBounds().intersects(sb->getGlobalBounds())){

                    thisPlayer->resolveColision(sb);
                    //LATERAL SHIFT

                    if(sb->side == RIGHT_SIDE){
                        //shift everything left
                        translationString = TRANSFORM_LEFT + " " + std::to_string(thisPlayer->id) + " " + std::to_string(frameDelta);
                    }
                    else{
                        //shift everything right
                        translationString = TRANSFORM_RIGHT + " " + std::to_string(thisPlayer->id) + " " + std::to_string(frameDelta);
                    }
                    sb->isCollidedWith = true;

                }
            }

            // std::cout << "RIGHT: " << std::to_string(boundaries[0]->isCollidedWith) << std::endl;
            // std::cout << "LEFT: " << std::to_string(boundaries[1]->isCollidedWith) << std::endl;

            if(boundaries[0]->isCollidedWith || boundaries[1]->isCollidedWith){
                bool otherCollidedWith = false;
                for(Player * pl : players){
                    if(pl->id != thisId){
                        if(boundaries[0]->isCollidedWith){
                            //check the other one
                            if(pl->getPosition().x <= boundaries[1]->getPosition().x + 1){
                                otherCollidedWith = true;
                                break;
                            }
                        }
                        else{
                            if(pl->getPosition().x + pl->getSize().x >= boundaries[0]->getPosition().x){
                                otherCollidedWith = true;
                                break;
                            }   
                        }
                    }
                }

                if(!otherCollidedWith){
                    zmq::message_t translateMessage(translationString.length());
                    memcpy(translateMessage.data(), translationString.c_str(), translationString.length());
                    newPlayerSocket.send(translateMessage, zmq::send_flags::none);
                    zmq::message_t rep(0);
                    newPlayerSocket.recv(rep, zmq::recv_flags::none); 
                }
            }
        }

        //update server on player's position
        std::string playerPosString = std::to_string(thisPlayer->id) + " " + std::to_string(thisPlayer->getPosition().x) + " " + std::to_string(thisPlayer->getPosition().y);
        zmq::message_t posMessage(playerPosString.length());
        memcpy(posMessage.data(), playerPosString.c_str(), playerPosString.length());

        newPlayerSocket.send(posMessage, zmq::send_flags::none);
        zmq::message_t rep(0);
        newPlayerSocket.recv(rep, zmq::recv_flags::none);     

        {
            std::lock_guard<std::mutex> lock(platformMutex);
            for(auto const& obj : gameObjects){
                window.draw(*obj.second);
            }
        }

        //DRAW BOUNDARIES (REMOVE LATER)
        //window.draw(*boundaries[0]);
        //window.draw(*boundaries[1]);

        window.display();
    }
}