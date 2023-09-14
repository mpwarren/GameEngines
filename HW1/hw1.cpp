#include <SFML/Graphics.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include <iostream>

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


    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        // window.draw(...);
        
        /*
        sf::CircleShape shape(50.f);

        shape.setFillColor(sf::Color(100, 250, 50));
        shape.setOutlineThickness(10.f);
        shape.setOutlineColor(sf::Color(250, 150, 100));
        */
        
        window.draw(platform);


        horzPlatform.movePosition();
        player.CheckCollision(horzPlatform);
        window.draw(horzPlatform);

        vertPlatform.movePosition();
        player.CheckCollision(vertPlatform);
        window.draw(vertPlatform);

        window.draw(player);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            player.movePlayer(sf::Keyboard::W, platform);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            player.movePlayer(sf::Keyboard::A, platform);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
            player.movePlayer(sf::Keyboard::S, platform);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            player.movePlayer(sf::Keyboard::D, platform);
        }

        // end the current frame
        window.display();
    }

    return 0;
}