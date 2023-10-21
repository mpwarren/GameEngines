#include <map>
#include "GameShapes/CollidableObject.h"
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include <zmq.hpp>
#include <mutex>
#include <thread>
#include "Timeline.h"
#include "SpawnPoint.h"


std::mutex platformMutex;

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
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
            //std::cout << "SETTING: " << updateString << std::endl;
            gameObjects->at(stoi(words[0]))->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
        }
    }
}


void playerPositionUpdates(std::map<int, CollidableObject*>* gameObjects, int thisId){
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
            std::cout << "NEW PLAYER" << std::endl;
            int id = stoi(words[1]);
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                gameObjects->insert(std::pair<int, CollidableObject*>(id, new Player(id, sf::Vector2f(stof(words[2]), stof(words[3])), sf::Vector2f(stof(words[4]), stof(words[5])), words[6])));
            }
        }
        else if(words[0] == DELETE_SIGN){
            {
                std::lock_guard<std::mutex> lock(platformMutex);
                int id = stoi(words[1]);
                delete gameObjects->at(id);
                gameObjects->erase(id);              
            }
        }
        else{
            int currentId = stoi(words[0]);
            if(currentId != thisId){
                //std::cout << "Non player position recieved: " << updateString << std::endl;
                //std::cout << "MOVING PLAYER " << updateString << std::endl;
                {
                    std::lock_guard<std::mutex> lock(platformMutex);
                    gameObjects->at(currentId)->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
                }

            }
        }
    }    
}


int main(){

    //set spawnpoint to a base value for now so player can be made
    SpawnPoint sp(sf::Vector2f(0,0));

    std::map<int, CollidableObject*> gameObjects;
    std::vector<CollidableObject*> collidableObjects;

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
    std::cout << "ID: " << idMessage.to_string() << std::endl;

    int moreToRead = 1;
    while(moreToRead != 0){
        zmq::message_t objectMessage;
        newPlayerSocket.recv(objectMessage, zmq::recv_flags::none);
        std::vector<std::string> params = parseMessage(objectMessage.to_string());
        
        std::cout << "Message: " << objectMessage.to_string() << std::endl;
        if(params[0] == PLATFORM_ID){
            int id = stoi(params[1]);
            Platform* pt = new Platform(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6]);
            gameObjects[id] = pt;
            collidableObjects.push_back(pt);
        }
        else if(params[0] == MOVING_PLATFORM_ID){
            int id = stoi(params[1]);
            MovingPlatform* mp = new MovingPlatform(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6], (Direction)stoi(params[7]), stof(params[8]), stoi(params[9]));
            gameObjects[id] = mp;
            collidableObjects.push_back(mp);
        }
        else if(params[0] == PLAYER_ID){
            int id = stoi(params[1]);
            gameObjects[id] = new Player(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6]);
        }
        else if(params[0] == SPAWN_POINT_ID){
            sp = SpawnPoint(sf::Vector2f(stof(params[1]), stof(params[2])));
        }
        else{
            std::cout << "ERROR: UNKNOWN OBJECT TYPE RECIEVED FROM SERVER" << std::endl;
        }

        size_t moreSize = sizeof(moreToRead);
        newPlayerSocket.getsockopt(ZMQ_RCVMORE, &moreToRead, &moreSize);
    }

    Player * thisPlayer = (Player*)gameObjects[thisId];

    for(int i = 1; i <= 3; i++){
        std::cout << gameObjects[i]->toString() << std::endl;
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "My Window", sf::Style::Default);

    std::thread platformThread(platformMovement, &gameObjects);
    std::thread playerThread(playerPositionUpdates, &gameObjects, thisId);


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

        zmq::message_t pauseListenerMessage(1);
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

        window.clear(sf::Color::Black);


        int64_t currentTime = gameTime.getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;

        bool moved = false;
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            thisPlayer->setJumping();
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            thisPlayer->movePlayer(sf::Keyboard::A, frameDelta);
            moved = true;
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            thisPlayer->movePlayer(sf::Keyboard::D, frameDelta);
            moved = true;
        }

        //gravity
        thisPlayer->gravity(frameDelta);
        moved = true;

        //collision
        {
            std::lock_guard<std::mutex> lock(platformMutex);
            for(CollidableObject* obj : collidableObjects){

                if(thisPlayer->getGlobalBounds().intersects(obj->getGlobalBounds())){
                    thisPlayer->resolveColision(obj);
                    thisPlayer->endJump();
                    moved = true;
                }
            }
        }



        if(moved){
            //update server on player's position
            std::string playerPosString = std::to_string(thisPlayer->id) + " " + std::to_string(thisPlayer->getPosition().x) + " " + std::to_string(thisPlayer->getPosition().y);
            zmq::message_t posMessage(playerPosString.length());
            memcpy(posMessage.data(), playerPosString.c_str(), playerPosString.length());

            newPlayerSocket.send(posMessage, zmq::send_flags::none);
            zmq::message_t rep(0);
            newPlayerSocket.recv(rep, zmq::recv_flags::none);            
        }

        {
            std::lock_guard<std::mutex> lock(platformMutex);
            for(auto const& obj : gameObjects){
                window.draw(*obj.second);
            }
        }

        
        window.display();
    }

    //remove player from game
    std::string playerPosString = DELETE_SIGN + " " + std::to_string(thisPlayer->id);
    zmq::message_t posMessage(playerPosString.length());
    memcpy(posMessage.data(), playerPosString.c_str(), playerPosString.length());

    newPlayerSocket.send(posMessage, zmq::send_flags::none);
    zmq::message_t rep(0);
    newPlayerSocket.recv(rep, zmq::recv_flags::none); 
}