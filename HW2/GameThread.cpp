#include "GameThread.h"

GameThread::GameThread(int i, ThreadExample *other, std::mutex *_mutex, std::condition_variable *_condition_variable){
    this->i = i; // set the id of this thread
    if(i==0) { busy = true; }
    else { this->other = other; }
    this->_mutex = _mutex;
    this->_condition_variable = _condition_variable;
}

bool GameThread::isBusy(){

}

void GameThread::run(){

}

void GameThread::PlayerThread(){

}

void GameThread::PlatformThread()

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "My Window", sf::Style::Default);

    //Create platforms and player
    Platform platform(sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");

    MovingPlatform horzPlatform(sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.5, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));

    MovingPlatform vertPlatform(sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.5, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));

    Player player(sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player.setFillColor(sf::Color(150, 50, 250));
}