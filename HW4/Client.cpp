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
            thisPlayer->gravity(1);
        }
    }
}


// void playerPositionUpdates(std::map<int, CollidableObject*>* gameObjects, std::vector<Player*>* players, int thisId){
//     zmq::context_t context (1);
//     zmq::socket_t recievePlayerPositionSocket (context, zmq::socket_type::sub);
//     recievePlayerPositionSocket.connect ("tcp://localhost:5558");
//     recievePlayerPositionSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

//     while(true){
//         zmq::message_t positionUpdate;
//         recievePlayerPositionSocket.recv(positionUpdate, zmq::recv_flags::none);
//         std::vector<std::string> words = parseMessage(positionUpdate.to_string());
        
//         //std::cout <<"PLAYER MOVEMENT: " << positionUpdate.to_string() << std::endl;

//         int currentId = stoi(words[0]);
//         if(currentId != thisId){
//             std::cout << "Recieved new position: " << positionUpdate.to_string() << std::endl;
//             {
//                 std::lock_guard<std::mutex> lock(dataMutex);
//                 if(gameObjects->count(currentId) != 0){
//                     std::cout << "SETTING POSITION\n";
//                     gameObjects->at(currentId)->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
//                 }
//             }
//         }

        
//     }    
// }

//listen for events published by the server
void eventListner(EventManager * em, Timeline * timeline){
    zmq::context_t context(1);
    zmq::socket_t eventListner(context, zmq::socket_type::sub);
    eventListner.connect("tcp://localhost:5559");
    eventListner.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t eventMessage;
        eventListner.recv(eventMessage, zmq::recv_flags::none);

        //std::cout << "EVENT MESSAGE: " << eventMessage.to_string() << std::endl;

        std::vector<std::string> params = parseMessage(eventMessage.to_string());

        EventType t = (EventType)stoi(params[0]);

        if(t == ADD_OTHER_PLAYER){

            std::string playerString = "";
            for(int i = 3; i <= 11; i++ ){
                playerString += params[i] + " ";
            }

            std::shared_ptr<AddOtherPlayerEvent> e = std::make_shared<AddOtherPlayerEvent>(timeline->getTime(), (Priority)stoi(params[2]), playerString);
            //std::cout << "ADDED NEW PLAYER EVENT TO QUEUE: " << e->toString() << std::endl;
            em->addToQueue(e);
        }
        else if(t == MOVE_PLAYER_EVENT){
            std::shared_ptr<UpdatePlayerPositionEvent> e = std::make_shared<UpdatePlayerPositionEvent>(timeline->getTime(), (Priority)stoi(params[1]), stoi(params[2]), stof(params[3]), stof(params[4]));
            em->addToQueue(e);
        }
        else if(t == TRANSLATE){
            std::shared_ptr<TranslationEvent> e = std::make_shared<TranslationEvent>(timeline->getTime(), (Priority)stoi(params[2]), params[3][0], stoi(params[4]), stoi(params[5]));
            em->addToQueue(e);
        }
    }


}


int main(){

    //set spawnpoint to a base value for now so player can be made
    SpawnPoint sp(sf::Vector2f(0,0), sf::Vector2f(0,0));

    std::map<int, CollidableObject*> gameObjects;
    std::vector<CollidableObject*> collidableObjects;
    std::vector<DeathZone*> deathZones;
    std::vector<Player*> otherPlayers;

    //Connect to server
    zmq::context_t context (3);
    zmq::socket_t newPlayerSocket (context, zmq::socket_type::req);
    newPlayerSocket.connect ("tcp://localhost:5556");

    zmq::socket_t eventSender(context, zmq::socket_type::push);
    eventSender.connect("tcp://localhost:5560");

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
            if(id != thisId){
                otherPlayers.push_back((Player*)gameObjects[id]);
            }
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
    int distanceFromEdge = 50;
    SideBoundry* rightBoundry = new SideBoundry(100, sf::Vector2f(SCENE_WIDTH - distanceFromEdge , -50), sf::Vector2f(SCENE_WIDTH - distanceFromEdge , -50), RIGHT_SIDE);
    SideBoundry* leftBoundry = new SideBoundry(101, sf::Vector2f(distanceFromEdge, -50), sf::Vector2f(distanceFromEdge, -50), LEFT_SIDE);

    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);
    int64_t lastTime = gameTime.getTime();

    //create event Handlers
    EventManager *eventManager = new EventManager();
    PlayerHandler * playerHandler = new PlayerHandler(&dataMutex, &gameObjects);
    WorldHandler * worldHandler = new WorldHandler(&dataMutex, &gameObjects, &gameTime);
    eventManager->addHandler(std::vector<EventType>{INPUT_MOVEMENT, GRAVITY, COLLISION_EVENT, SPAWN_EVENT}, playerHandler);
    eventManager->addHandler(std::vector<EventType>{ADD_OTHER_PLAYER, MOVE_PLAYER_EVENT, DEATH_EVENT, TRANSLATE}, worldHandler);

    std::thread platformThread(platformMovement, &gameObjects, thisPlayer);
    //std::thread playerThread(playerPositionUpdates, &gameObjects, &players, thisId);
    std::thread eventListnerThread(eventListner, eventManager, &gameTime);
    //std::thread heartbeatThread(heartbeat, thisId);

    //spawn the player
    eventManager->addToQueue(std::make_shared<SpawnEvent>(0, HIGH, thisId, &sp));

        
    float prevX = 0;
    float prevY = 0;


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

            if(thisPlayer->gravity(frameDelta)){
                //lock.unlock();
                std::shared_ptr<GravityEvent> e = std::make_shared<GravityEvent>(currentTime, HIGH, thisId);
                eventManager->addToQueue(e);
            }

            bool died = false;
            //lock.lock();
            for(DeathZone * dz : deathZones){
                if(thisPlayer->checkCollision(dz)){
                    std::shared_ptr<DeathEvent> e = std::make_shared<DeathEvent>(currentTime, LOW);
                    died = true;
                    eventManager->addToQueue(e);

                    std::string eventStr = e->toString();
                    zmq::message_t deathEventMsg(eventStr.length());
                    memcpy(deathEventMsg.data(), eventStr.c_str(), eventStr.length());
                    eventSender.send(deathEventMsg, zmq::send_flags::none);
                }
            }

            if(!died){

                //check for translation
                bool shouldTranslate = false;
                char direction;
                if(thisPlayer->checkCollision(leftBoundry)){
                    thisPlayer->resolveColision(leftBoundry);
                    shouldTranslate = true;
                    direction = 'R';
                    for(Player * p : otherPlayers){
                        if(p->getPosition().x + p->getSize().x >= rightBoundry->getPosition().x){
                            shouldTranslate = false;
                            break;
                        }
                    }
                }
                else if(thisPlayer->checkCollision(rightBoundry)){
                    thisPlayer->resolveColision(rightBoundry);
                    shouldTranslate = true;
                    direction = 'L';
                    for(Player * p : otherPlayers){
                        if(p->getPosition().x <= leftBoundry->getPosition().x + 1){
                            shouldTranslate = false;
                            break;
                        }
                    }
                }

                if(shouldTranslate){
                    std::cout << "SENDING TRANSLATE EVENT\n";
                    std::string translationEventString = std::to_string(TRANSLATE) + " 0 " + std::to_string(HIGH) + " " + direction + " " + std::to_string(thisId) + " " + std::to_string(frameDelta);
                    zmq::message_t translationMessage(translationEventString.length());
                    memcpy(translationMessage.data(), translationEventString.c_str(), translationEventString.length());
                    eventSender.send(translationMessage, zmq::send_flags::none);
                }


                bool collided = false;
                for(CollidableObject * co : collidableObjects){
                    if(thisPlayer->checkCollision(co)){
                        std::shared_ptr<CollisionEvent> e = std::make_shared<CollisionEvent>(currentTime, MEDIUM, thisId, co->id);
                        //lock.unlock();
                        collided = true;
                        eventManager->addToQueue(e);
                        //lock.lock();
                    }
                }
                thisPlayer->setColliding(collided);
                if(!collided){
                    thisPlayer->setIsCollidingUnder(false);
                }
            }


            //-----------------------------------------------------------------

            lock.unlock();
            //Process Events
            {
                std::lock_guard<std::mutex> lock(eventManager->mutex);
                while(!eventManager->eventQueueHigh.empty() && eventManager->eventQueueHigh.top()->timeStamp <= gameTime.getTime()){
                    std::shared_ptr<Event> ev = eventManager->eventQueueHigh.top();
                    for(EventHandler* h : eventManager->handlers[ev->eventType]){
                        h->onEvent(ev);
                    }
                    eventManager->eventQueueHigh.pop();
                }
                while(!eventManager->eventQueueMedium.empty() && eventManager->eventQueueMedium.top()->timeStamp <= gameTime.getTime()){
                    std::shared_ptr<Event> ev = eventManager->eventQueueMedium.top();
                    for(EventHandler* h : eventManager->handlers[ev->eventType]){
                        h->onEvent(ev);
                    }
                    eventManager->eventQueueMedium.pop();
                }
                while(!eventManager->eventQueueLow.empty() && eventManager->eventQueueLow.top()->timeStamp <= gameTime.getTime()){
                    std::shared_ptr<Event> ev = eventManager->eventQueueLow.top();
                    for(EventHandler* h : eventManager->handlers[ev->eventType]){
                        h->onEvent(ev);
                    }
                    eventManager->eventQueueLow.pop();
                }
            }

            lock.lock();


            //send the server the current player position
            if(thisPlayer->getPosition().x != prevX || thisPlayer->getPosition().y != prevY){
                std::string playerPosEvent = std::to_string((int)MOVE_PLAYER_EVENT) + " " + std::to_string((int)MEDIUM) + " " + std::to_string(thisPlayer->id) + " " + std::to_string(thisPlayer->getPosition().x) + " " + std::to_string(thisPlayer->getPosition().y);
                prevX = thisPlayer->getPosition().x;
                prevY = thisPlayer->getPosition().y;
                zmq::message_t posMsg(playerPosEvent.length());
                memcpy(posMsg.data(), playerPosEvent.c_str(), playerPosEvent.length());
                eventSender.send(posMsg, zmq::send_flags::none);

            }


            //---------------------------------------------------------------

            for(auto const& obj : gameObjects){
                window.draw(*obj.second);
            }
            //lock.unlock();
        }

        window.display();
    }
}