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
#include <v8.h>
#include "ScriptManager.h"
#include <libplatform/libplatform.h>
#include "v8helpers.h"


std::mutex dataMutex;
EventManager *eventManager = new EventManager();
Timeline anchorTimeline;
Timeline gameTime(&anchorTimeline);

void ScriptEventGenerator(const v8::FunctionCallbackInfo<v8::Value>& args){
	v8::Isolate *isolate = args.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::EscapableHandleScope handle_scope(args.GetIsolate());
	v8::Context::Scope context_scope(context);

    std::shared_ptr<Event> dm = std::make_shared<CollisionEvent>(gameTime.getTime(), LOW, 8, 7);
    eventManager->addToQueue(dm);
    //v8::Local<v8::Object> v8_obj = std::dynamic_pointer_cast<DeathEvent>(dm)->exposeToV8(isolate, context);
	//args.GetReturnValue().Set(handle_scope.Escape(v8_obj));
}

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void heartbeat(int id){
    //Connect to server
    zmq::context_t context (1);
    zmq::socket_t heartbeatSocket (context, zmq::socket_type::push);
    heartbeatSocket.connect ("tcp://localhost:5561");
    std::string idString = std::to_string(id);
    int strLen = idString.length();


    while(true){
        zmq::message_t idMessage(strLen);
        memcpy(idMessage.data(), idString.c_str(), strLen);

        heartbeatSocket.send(idMessage, zmq::send_flags::none);

        sleep(2);
    }


}

void platformMovement(std::map<int, CollidableObject*>* gameObjects, Player* thisPlayer){
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t platformReciever (context, zmq::socket_type::sub);
    platformReciever.connect ("tcp://localhost:5555");
    platformReciever.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t positionUpdate;
        //std::cout <<"WAITING ON PLATFORM UPDATE" << std::endl;
        zmq::recv_result_t r = platformReciever.recv(positionUpdate, zmq::recv_flags::none);
        std::vector<std::string> words = parseMessage(positionUpdate.to_string());
        int id = stoi(words[0]);
        if(id != thisPlayer->id){
            std::lock_guard<std::mutex> lock(dataMutex);
            //std::cout << "SETTING1: " << positionUpdate << std::endl;
            if(gameObjects->count(id) == 1){
                gameObjects->at(stoi(words[0]))->setPosition(sf::Vector2f(stof(words[1]), stof(words[2])));
            }
        }
    }
}

//listen for events published by the server
void eventListner(EventManager * em, Timeline * timeline){
    zmq::context_t context(1);
    zmq::socket_t eventListner(context, zmq::socket_type::sub);
    eventListner.connect("tcp://localhost:5559");
    eventListner.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t eventMessage;
        zmq::recv_result_t r = eventListner.recv(eventMessage, zmq::recv_flags::none);

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
        else if(t == DEATH_EVENT){
            std::shared_ptr<DeathEvent> e = std::make_shared<DeathEvent>(timeline->getTime(), (Priority)stoi(params[2]));
            em->addToQueue(e);
        }
        else if(t == TRANSLATE){
            std::shared_ptr<TranslationEvent> e = std::make_shared<TranslationEvent>(timeline->getTime(), (Priority)stoi(params[2]), params[3][0], stoi(params[4]), stoi(params[5]));
            em->addToQueue(e);
        }
        else if(t == REMOVE_PLAYER){
            std::shared_ptr<RemovePlayerEvent> e = std::make_shared<RemovePlayerEvent>(timeline->getTime(), (Priority)stoi(params[2]),stoi(params[3]));
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
    zmq::recv_result_t r =newPlayerSocket.recv(idMessage, zmq::recv_flags::none);
    std::cout << idMessage.to_string() << std::endl;
    int thisId = stoi(idMessage.to_string());
    std::cout << "Connected with ID: " << idMessage.to_string() << std::endl;

    int numPlatforms = 0;
    std::vector<sf::Color> platformColors{ sf::Color(252, 186, 3), sf::Color(53, 252, 3), sf::Color(3, 78, 252)};
    int moreToRead = 1;
    while(moreToRead != 0){
        zmq::message_t objectMessage;
        zmq::recv_result_t r =newPlayerSocket.recv(objectMessage, zmq::recv_flags::none);
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
            collidableObjects.push_back(dz);
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
    PlayerHandler * playerHandler = new PlayerHandler(&dataMutex, &gameObjects);
    ClientWorldHandler * worldHandler = new ClientWorldHandler(&dataMutex, &gameObjects, &gameTime, thisId);
    eventManager->addHandler(std::vector<EventType>{INPUT_MOVEMENT, COLLISION_EVENT, GRAVITY, SPAWN_EVENT}, playerHandler);
    eventManager->addHandler(std::vector<EventType>{ADD_OTHER_PLAYER, DEATH_EVENT, TRANSLATE, REMOVE_PLAYER}, worldHandler);

    std::thread platformThread(platformMovement, &gameObjects, thisPlayer);
    //std::thread playerThread(playerPositionUpdates, &gameObjects, &players, thisId);
    std::thread eventListnerThread(eventListner, eventManager, &gameTime);
    std::thread heartbeatThread(heartbeat, thisId);

    //spawn the player
    eventManager->addToQueue(std::make_shared<SpawnEvent>(0, HIGH, thisId, &sp));

    //INITALIZE SCRIPTING
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.release());
    v8::V8::InitializeICU();
    v8::V8::Initialize();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);

    {
        v8::Isolate::Scope isolate_scope(isolate); // must enter the virtual machine to do stuff
        v8::HandleScope handle_scope(isolate);

        // Best practice to isntall all global functions in the context ahead of time.
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

        // Bind the global 'print' function to the C++ Print callback.
        global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, v8helpers::Print));
        
        // Bind the global static function for retrieving object handles
        global->Set(isolate, "gethandle", v8::FunctionTemplate::New(isolate, ScriptManager::getHandleFromScript));

		global->Set(isolate, "eventFactory", v8::FunctionTemplate::New(isolate, ScriptEventGenerator));


        v8::Local<v8::Context> default_context =  v8::Context::New(isolate, NULL, global);
        v8::Context::Scope default_context_scope(default_context); // enter the context

        ScriptManager *sm = new ScriptManager(isolate, default_context); 

        //Create Player Context
        v8::Local<v8::Context> player_context = v8::Context::New(isolate, NULL, global);
		sm->addContext(isolate, player_context, "player_context");

        //expose player to v8
        thisPlayer = (Player*) gameObjects[thisId];
        thisPlayer->setOutlineThickness(1);
        thisPlayer->exposeToV8(isolate, player_context);

        //ADD SCRIPTS
        sm->addScript("change_color", "scripts/change_color.js", "player_context");
        sm->addScript("check_death_event", "scripts/raise_death_event.js", "player_context");
        sm->addScript("death_color", "scripts/death_color.js", "player_context");

        ScriptHandler * scriptHandler = new ScriptHandler(&dataMutex, &gameObjects, sm);
        eventManager->addHandler(std::vector<EventType>{DEATH_EVENT}, scriptHandler);


        while(window.isOpen()){
            
            // check all the window's events that were triggered since the last iteration of the loop
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::KeyPressed){
                    if(event.key.code == sf::Keyboard::P){
                        if(gameTime.isPaused()){
                            int64_t elapsedTime = gameTime.unpause();
                            lastTime = gameTime.getTime();
                        }
                        else{
                            gameTime.pause(lastTime);
                        }  
                    }
                    else if(event.key.code == sf::Keyboard::Z){
                        gameTime.changeTic(TIC_HALF);
                        lastTime = gameTime.getTime();
                    }
                    else if(event.key.code == sf::Keyboard::X){
                        gameTime.changeTic(TIC_NORMAL);
                        lastTime = gameTime.getTime();
                    }
                    else if(event.key.code == sf::Keyboard::C){
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
                std::shared_ptr<MovementInputEvent> e = std::make_shared<MovementInputEvent>(currentTime, HIGH, thisId, 'W', frameDelta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                eventManager->addToQueue(e);
            }
            if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
                std::shared_ptr<MovementInputEvent> e = std::make_shared<MovementInputEvent>(currentTime, HIGH, thisId, 'A', frameDelta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                eventManager->addToQueue(e);
            }
            if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
                std::shared_ptr<MovementInputEvent> e = std::make_shared<MovementInputEvent>(currentTime, HIGH, thisId, 'D', frameDelta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
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
                    //std::cout << "SENDING TRANSLATE EVENT\n";
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

                //std::cout << std::to_string(thisPlayer->getPosition().y + 50) << std::endl;

                sm->runOne("change_color", false, "player_context");
                sm->runOne("check_death_event", false, "player_context");

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


                for(auto const& obj : gameObjects){
                    window.draw(*obj.second);
                }
            }

            window.display();
        }
    }

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();

	return 0;
}