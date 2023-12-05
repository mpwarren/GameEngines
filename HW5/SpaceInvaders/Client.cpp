#include <map>
#include "CollidableObject.h"
#include "Player.h"
#include <zmq.hpp>
#include <mutex>
#include <thread>
#include "Timeline.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"
#include <unistd.h>
#include "Bullet.h"
#include "EnemyGrid.h"
#include "EnemyBullet.h"
#include <v8.h>
#include "ScriptManager.h"
#include <libplatform/libplatform.h>
#include "v8helpers.h"

std::mutex *enemyMutex;
std::mutex *enemyBulletMutex;

//create timelines
Timeline anchorTimeline;
Timeline gameTime(&anchorTimeline);

EventManager *eventManager = new EventManager();

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void ScriptEventGenerator(const v8::FunctionCallbackInfo<v8::Value>& args){
	v8::Isolate *isolate = args.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::EscapableHandleScope handle_scope(args.GetIsolate());
	v8::Context::Scope context_scope(context);


    std::shared_ptr<Event> e = std::make_shared<GainLifeEvent>(gameTime.getTime(), LOW);
    eventManager->addToQueue(e, false);
    //v8::Local<v8::Object> v8_obj = std::dynamic_pointer_cast<DeathEvent>(dm)->exposeToV8(isolate, context);
	//args.GetReturnValue().Set(handle_scope.Escape(v8_obj));
}

void recieveEnemyInstructions(EnemyGrid* en, std::vector<std::shared_ptr<EnemyBullet>>* enemyBullets){
    //connect to server
    zmq::context_t context (1);
    zmq::socket_t enemyListner (context, zmq::socket_type::pull);
    enemyListner.connect ("tcp://localhost:5557");

    while(true){
        zmq::message_t aiMessage(10);
        zmq::recv_result_t r = enemyListner.recv(aiMessage, zmq::recv_flags::none);

        std::vector<std::string> params = parseMessage(aiMessage.to_string());

        if(params[0] == "S"){
            std::shared_ptr<EnemyBullet> b;
            {
                std::lock_guard<std::mutex> lock(*enemyMutex);
                Enemy * enemy = en->enemyGrid[stoi(params[1])][stoi(params[2])];
                b = std::make_shared<EnemyBullet>(enemy->getPosition().x + (enemy->getSize().x / 2), enemy->getPosition().y);
            }
            {
                std::lock_guard<std::mutex> lock(*enemyBulletMutex);
                enemyBullets->push_back(b);
            }
        }
    }
}

int main(){

    //mutexes
    enemyMutex = new std::mutex();
    enemyBulletMutex = new std::mutex();

    //game objects and data structures
    Player * player;
    std::vector<std::shared_ptr<Bullet>> bullets;
    std::vector<std::shared_ptr<EnemyBullet>> enemyBullets;

    sf::Text livesText;
    livesText.setString("Lives:");
    sf::Font font;
    font.loadFromFile("Roboto-Black.ttf");
    livesText.setFont(font);
    livesText.setCharacterSize(20);
    livesText.setPosition(10, 560);

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setPosition(650, 10);

    sf::Text endText;
    endText.setFont(font);
    endText.setCharacterSize(40);
    endText.setPosition(SCENE_WIDTH / 2 - 100, SCENE_HEIGHT / 2 - 20);

    sf::RectangleShape lifeMarker(sf::Vector2f(20, 20));
    lifeMarker.setFillColor(sf::Color(255, 0, 0));
    EnemyGrid *enemies;

    //connect to server
    zmq::context_t context (1);
    zmq::socket_t newPlayerSocket (context, zmq::socket_type::req);
    newPlayerSocket.connect ("tcp://localhost:5556");

    zmq::message_t newConnection(0);
    newPlayerSocket.send(newConnection, zmq::send_flags::none);

    //get id
    zmq::message_t idMessage(2);
    zmq::recv_result_t r =newPlayerSocket.recv(idMessage, zmq::recv_flags::none);
    std::cout << idMessage.to_string() << std::endl;
    int thisId = stoi(idMessage.to_string());
    std::cout << "Connected with ID: " << idMessage.to_string() << std::endl;

    //get game objects from server
    int moreToRead = 1;
    while(moreToRead != 0){
        zmq::message_t objectMessage;
        zmq::recv_result_t r =newPlayerSocket.recv(objectMessage, zmq::recv_flags::none);
        std::vector<std::string> params = parseMessage(objectMessage.to_string());

        if(params[0] == PLAYER_ID){
            int id = stoi(params[1]);
            player = new Player(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6]);
        }
        else if(params[0] == ENEMY_GRID_ID){
            enemies = new EnemyGrid(stoi(params[1]), stoi(params[2]), stoi(params[3]));
        }
        else{
            std::cout << "ERROR: UNKNOWN OBJECT TYPE RECIEVED FROM SERVER" << std::endl;
        }

        size_t moreSize = sizeof(moreToRead);
        newPlayerSocket.getsockopt(ZMQ_RCVMORE, &moreToRead, &moreSize);
    }

    //create event manager and handlers

    PlayerHandler * playerHandler = new PlayerHandler(player);
    eventManager->addHandler(std::vector<EventType>{MOVEMENT_EV, ENEMY_DEATH_EV, GAIN_LIFE_EV}, playerHandler);

    EnemyHandler * enemyHandler = new EnemyHandler(enemies, enemyMutex);
    eventManager->addHandler(std::vector<EventType>{ENEMY_DEATH_EV}, enemyHandler);


    //create sfml window
    sf::RenderWindow window(sf::VideoMode(SCENE_WIDTH, SCENE_HEIGHT), "Space Invaders Clone", sf::Style::Default);


    int64_t lastTime = gameTime.getTime();

    std::thread enemyAIthread(recieveEnemyInstructions, enemies, &enemyBullets);

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

        player->exposeToV8(isolate, player_context);
        enemies->exposeToV8(isolate, player_context);
        sm->addScript("change_color", "scripts/change_color.js", "player_context");
        sm->addScript("gain_life", "scripts/gain_life.js", "player_context");

        ScriptHandler * scriptHandler = new ScriptHandler(sm);
        eventManager->addHandler(std::vector<EventType>{ENEMY_DEATH_EV}, scriptHandler);

        while(window.isOpen()){
        
            // check all the window's events that were triggered since the last iteration of the loop
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::KeyPressed){
                    if(event.key.code == sf::Keyboard::Space){
                        std::shared_ptr<Bullet> b = std::make_shared<Bullet>(player->getPosition().x);
                        bullets.push_back(b);
                    }
                    else if(event.key.code == sf::Keyboard::P){
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
            
            //scripts
            sm->runOne("change_color", false, "player_context");

            int64_t currentTime = gameTime.getTime();
            int64_t frameDelta = currentTime - lastTime;
            lastTime = currentTime;

            if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
                std::shared_ptr<MovementEvent> e = std::make_shared<MovementEvent>(currentTime, HIGH, 'A', frameDelta);
                eventManager->addToQueue(e);
            }
            if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
                std::shared_ptr<MovementEvent> e = std::make_shared<MovementEvent>(currentTime, HIGH, 'D', frameDelta);
                eventManager->addToQueue(e);
            }

            //move bullets and check for collisions with enemies
            for(auto it = bullets.begin(); it != bullets.end();){
                if((*it)->getPosition().y < 0){
                    it = bullets.erase(it);
                }
                else{
                    bool removed = false;
                    std::lock_guard<std::mutex> lock(*enemyMutex);
                    for(std::vector<Enemy*> row : enemies->enemyGrid){
                        for(Enemy* e : row){
                            if(!e->dead && (*it)->getGlobalBounds().intersects(e->getGlobalBounds())){
                                int row = e->row;
                                int col = e->col;
                                std::shared_ptr<EnemyDeathEvent> e = std::make_shared<EnemyDeathEvent>(currentTime, HIGH, row, col);
                                eventManager->addToQueue(e);
                                it = bullets.erase(it);
                                removed = true;
                            }
                        }
                    }
                    if(!removed){
                        (*it)->moveBullet(frameDelta);
                        ++it;
                    }
                }
            }

            {
                std::lock_guard<std::mutex> lock(*enemyBulletMutex);
                for(auto it = enemyBullets.begin(); it != enemyBullets.end();){
                    if((*it)->getGlobalBounds().intersects(player->getGlobalBounds())){
                        player->lives--;
                        it = enemyBullets.erase(it);
                    }
                    else if((*it)->getPosition().y > SCENE_HEIGHT){
                        it = enemyBullets.erase(it);
                    }
                    else{
                        (*it)->moveBullet(frameDelta);
                        ++it; 
                    }
                }
            }


            //move enemies
            {
                std::lock_guard<std::mutex> lock(*enemyMutex);
                enemies->moveEnemies(frameDelta);
            }

            //process events
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

            if(player->lives != 0){
                window.draw(*player);
            }
            for(std::shared_ptr<Bullet> b : bullets){
                window.draw(*b);
            }

            {
                std::lock_guard<std::mutex> lock(*enemyBulletMutex);
                for(std::shared_ptr<EnemyBullet> b : enemyBullets){
                    window.draw(*b);
                }
            }


            //draw enemies
            {
                std::lock_guard<std::mutex> lock(*enemyMutex);
                for(std::vector<Enemy*> row : enemies->enemyGrid){
                    for(Enemy* e : row){
                        if(!e->dead){
                            window.draw(*e);
                        }
                    }
                }
            }


            //draw lives 
            window.draw(livesText);
            lifeMarker.setPosition(sf::Vector2f(70, 570));
            for(int i = 0; i < player->lives; i++){
                window.draw(lifeMarker);
                lifeMarker.setPosition(lifeMarker.getPosition().x + 40, lifeMarker.getPosition().y);
            }

            scoreText.setString("Score: " + std::to_string(player->score));
            window.draw(scoreText);

            if(player->lives == 0){
                gameTime.pause(lastTime);
                endText.setString("Game Over");
                endText.setFillColor(sf::Color(255, 0, 0));
                window.draw(endText);
            }
            else if(enemies->numAlive == 0){
                gameTime.pause(lastTime);
                endText.setString("You Win!");
                endText.setFillColor(sf::Color(0, 255, 0));
                window.draw(endText);
            }

            window.display();

        }
    }

}