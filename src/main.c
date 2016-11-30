#include <SFML/Graphics.hpp>
#include "../include/boid.h" // les calculs sur les boids en openmp
#include <iostream>   //pour cout endl
#include <X11/Xlib.h> //pour XInitThreads
#include <functional> //pour bind
#include <time.h>  // pour srand time null
#include <stdlib.h>
#include <omp.h>
#include <stdint.h>

#define NBOIDS 500

void renderLoop(sf::RenderWindow * window, int nboids, float * xpositions, float * ypositions, float * rotations)
{

    //petit triangle
    sf::CircleShape shape(10.f,3);
    //shape.setFillColor(sf::Color::Green);
    shape.setOrigin(10.0f, 10.0f);
    shape.setPosition(50.0f,50.0f);

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
        for(int i = 0; i < nboids; i++)
        {
            shape.setPosition(xpositions[i], ypositions[i]);
            shape.setRotation(rotations[i]);
            window->draw(shape);
        }
        window->display();
    }

}

int main(int argc, char ** argv)
{
    uint32_t nboids = NBOIDS;
    float xpos[NBOIDS];
    float ypos[NBOIDS];
    float xvel[NBOIDS];
    float yvel[NBOIDS];
    float rot[NBOIDS];

    omp_set_num_threads(4);

    //Lancement du thread de rendu
    XInitThreads();
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flocking Simulation");
    window.setFramerateLimit(60);
    window.setActive(false);
    sf::Thread thread(std::bind(&renderLoop, &window, nboids, xpos, ypos, rot));
    thread.launch();

    //on garde les dimensions de la fenêtre dans un coin, ca peut être utile
    sf::Vector2u win_size = window.getSize();
    std::cout << "Actual window size is " <<  win_size.x << " "  << win_size.y << std::endl;

    srand(time(NULL)); // :D

    for(int i = 0; i < NBOIDS; i++)
    {
        xpos[i] = (float) (rand() % win_size.x +1);
        ypos[i] = (float) (rand() % win_size.y +1);
        rot[i] = (float) (rand() % 360 +1);
        xvel[i] = 0.0f;
        yvel[i] = 0.0f;
    }

    sf::Clock clock;
    sf::Time elapsed = clock.restart();
    const sf::Time update_ms = sf::seconds(1.f / 60.f);

    int toggle = 0;

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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            toggle = !toggle;

        elapsed += clock.restart();
        while (elapsed >= update_ms) {
            std::cout << xpos[1] << " " << ypos[1] << " " << xvel[1] << " " << yvel[1] << " " << rot[1] << std::endl;
            updateBoidsOpenMP(NBOIDS, update_ms.asSeconds(), xpos, ypos, xvel, yvel, rot);
            elapsed -= update_ms;
        }
    }

    return 0;
}

