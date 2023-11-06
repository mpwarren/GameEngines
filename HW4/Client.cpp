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
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"
#include <unistd.h>


std::mutex dataMutex;

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

// void heartbeat(int id){
//     //Connect to server
//     zmq::context_t context (1);
//     zmq::socket_t heartbeatSocket (context, zmq::socket_type::req);
//     heartbeatSocket.connect ("tcp://localhost:5559");
//     std::string idString = std::to_string(id);
//     int strLen = idString.length();


//     while(true){
//         zmq::message_t idMessage(strLen);
//         memcpy(idMessage.data(), idString.c_str(), strLen);

//         heartbeatSocket.send(idMessage, zmq::send_flags::none);
//         zmq::message_t recvMsg(0);
//         heartbeatSocket.recv(recvMsg, zmq::recv_flags::none);

//         sleep(3);
//     }


// }

void platformMovement(std::map<int, CollidableObject*>* gameObjects, Player* thisPlayer){
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t platformReciever (context, zmq::socket_type::sub);
    platformReciever.connect ("tcp://localhost:5555");
    platformReciever.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t positionUpdate;
        //std::cout <<"WAITING ON PLATFORM UPDATE" << std::endl;
        platformReciever.recv(positionUpdate, zmq::recv_flags::none);
        //std::cout <<"GOT PLATFORM UPDATE" << positionUpdate.to_string() << std::endl;
        std::vector<std::string> words = parseMessage(positionUpdate.to_string());
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            //std::cout << "SETTING1: " << positionUpdate << std::endl;
            gameObjects->at(stoi(words[0]))->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
            thisPlayer->gravity(1, (thisPlayer->getPosition().y + 50 >= 585));
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
        
        //std::cout <<"PLAYER MOVEMENT: " << positionUpdate.to_string() << std::endl;

        int currentId = stoi(words[0]);
        std::cout << "Recieved new position: " << positionUpdate.to_string() << std::endl;
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            gameObjects->at(currentId)->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
        }
        
    }    
}

void eventProcessor(EventManager * eventManager, Timeline* gameTimeline){
    while(true)
    {
        //Process Events
        std::lock_guard<std::mutex> lock(eventManager->mutex);
        while(!eventManager->eventQueueHigh.empty() && eventManager->eventQueueHigh.top()->timeStamp <= gameTimeline->getTime()){
            std::shared_ptr<Event> ev = eventManager->eventQueueHigh.top();
            for(EventHandler* h : eventManager->handlers[ev->eventType]){
                h->onEvent(ev);
            }
            eventManager->eventQueueHigh.pop();
        }
        while(!eventManager->eventQueueMedium.empty() && eventManager->eventQueueMedium.top()->timeStamp <= gameTimeline->getTime()){
            std::shared_ptr<Event> ev = eventManager->eventQueueMedium.top();
            for(EventHandler* h : eventManager->handlers[ev->eventType]){
                h->onEvent(ev);
            }
            eventManager->eventQueueMedium.pop();
        }
        while(!eventManager->eventQueueLow.empty() && eventManager->eventQueueLow.top()->timeStamp <= gameTimeline->getTime()){
             std::shared_ptr<Event> ev = eventManager->eventQueueLow.top();
            for(EventHandler* h : eventManager->handlers[ev->eventType]){
                h->onEvent(ev);
            }
            eventManager->eventQueueLow.pop();
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

    zmq::socket_t EventSender (context, zmq::socket_type::push);
    EventSender.connect ("tcp://localhost:5558");

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
            if(pt->id != 1){
                collidableObjects.push_back(pt);
            }
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
    // std::vector<SideBoundry*> boundaries;
    // int distanceFromEdge = 50;
    // SideBoundry* boundry1 = new SideBoundry(100, sf::Vector2f(SCENE_WIDTH - distanceFromEdge , -50), sf::Vector2f(SCENE_WIDTH - distanceFromEdge , -50), RIGHT_SIDE);
    // boundaries.push_back(boundry1);
    // SideBoundry* boundry2 = new SideBoundry(101, sf::Vector2f(distanceFromEdge, -50), sf::Vector2f(distanceFromEdge, -50), LEFT_SIDE);
    // boundaries.push_back(boundry2);

    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);
    int64_t lastTime = gameTime.getTime();

    //create event Handlers
    EventManager *eventManager = new EventManager();
    PlayerHandler * playerHandler = new PlayerHandler(&dataMutex, &gameObjects);
    eventManager->addHandler(std::vector<EventType>{INPUT_MOVEMENT, GRAVITY, COLLISION_EVENT}, playerHandler);

    std::thread platformThread(platformMovement, &gameObjects, thisPlayer);
    std::thread playerThread(playerPositionUpdates, &gameObjects, &players, thisId);
    std::thread eventProcessorThread(eventProcessor, eventManager, &gameTime);
    //std::thread heartbeatThread(heartbeat, thisId);



        



    while(window.isOpen()){
        
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::P){
                    //PAUSING CODE HERE
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
        window.clear();


        int64_t currentTime = gameTime.getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;

        //std::cout << "RUNNING" << std::to_string(lastTime);

        //MOVEMENT
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            std::shared_ptr<MovementInputEvent> e = std::make_shared<MovementInputEvent>(currentTime, HIGH, thisId, 'W', frameDelta);
            eventManager->addToQueue(e);
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            std::shared_ptr<MovementInputEvent> e = std::make_shared<MovementInputEvent>(currentTime, HIGH, thisId, 'A', frameDelta);
            eventManager->addToQueue(e);
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            std::shared_ptr<MovementInputEvent> e = std::make_shared<MovementInputEvent>(currentTime, HIGH, thisId, 'D', frameDelta);
            eventManager->addToQueue(e);
        }



        //GRAVITY/COLLISIONS/DRAWING
        {
            std::unique_lock<std::mutex> lock(dataMutex);

            if(thisPlayer->gravity(frameDelta, (thisPlayer->getPosition().y + 50 >= 585))){
                std::shared_ptr<GravityEvent> e = std::make_shared<GravityEvent>(currentTime, HIGH, thisId);
                lock.unlock();
                eventManager->addToQueue(e);
                lock.lock();
            }

            for(CollidableObject * co : collidableObjects){
                if(thisPlayer->checkCollision(co)){
                    std::shared_ptr<CollisionEvent> e = std::make_shared<CollisionEvent>(currentTime, MEDIUM, thisId, co->id);
                    lock.unlock();
                    eventManager->addToQueue(e);
                    lock.lock();
                }
            }

            for(auto const& obj : gameObjects){
                window.draw(*obj.second);
            }
        }

        window.display();
    }
}