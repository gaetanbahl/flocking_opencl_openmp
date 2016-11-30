#include <SFML/Graphics.hpp>
#include "../include/boiddrawing.h"
#include <iostream>
#include <X11/Xlib.h>



void renderLoop(sf::RenderWindow * window)
{

    //petit triangle
    sf::CircleShape shape(10.f,3);
    //shape.setFillColor(sf::Color::Green);

    //chargement de la petite texture pour le triangle
    sf::Texture txtr;
    if(!txtr.loadFromFile("images/texture.png"))
    {
        std::cout << "could not load texture, exiting program" << std::endl;
        return;
    }
    else
    {
        std::cout << "successfully loaded texture" << std::endl;
    }

    shape.setTexture(&txtr);

    //boucle de rendu
    while (window->isOpen())
    {
        window->clear();
        window->draw(shape);
        window->display();
    }

}


int main(int argc, char ** argv)
{
    //Lancement du thread de rendu
    XInitThreads();
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flocking Simulation");
    window.setActive(false);
    sf::Thread thread(&renderLoop, &window);
    thread.launch();

    //on garde les dimensions de la fenêtre dans un coin, ca peut être utile
    sf::Vector2u win_size = window.getSize();
    std::cout << "Actual window size is " <<  win_size.x << " "  << win_size.y << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            std::cout << "exiting program" << std::endl;
            window.close();
        }
    }

    return 0;
}

