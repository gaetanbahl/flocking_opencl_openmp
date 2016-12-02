#include <SFML/Graphics.hpp>
#include "../include/boid.h" // les calculs sur les boids en openmp
#include <iostream>   //pour cout endl
#include <X11/Xlib.h> //pour XInitThreads
#include <functional> //pour bind
#include <time.h>  // pour srand time null
#include <stdlib.h>
#include <omp.h>
#include <stdint.h>
#include <math.h>
#include <libconfig.h++>


using namespace libconfig;

inline float calcRot(float x, float y)
{
        if(norm(x,y) != 0.0f)
        {
            if(x > 0 )
            {
                return 180.0f - 180.0f / 3.1415 * acos(y/norm(x,y));
            }
            else
            {
                return -180.0f + 180.0f / 3.1415 * acos(y/norm(x,y));
            }
        }
        return 0.0f;
}

void renderLoop(sf::RenderWindow * window, int nboids, float * xpositions, float * ypositions, float * xvel, float * yvel)
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

    //load font
    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf"))
    {
        std::cout << "couldn't load font, exiting program" << std::endl;
        return;
    }

    //boucle de rendu
    while (window->isOpen())
    {
        window->clear();
        for(int i = 0; i < nboids; i++)
        {
            shape.setPosition(xpositions[i], ypositions[i]);
            shape.setRotation(calcRot(xvel[i], yvel[i]));
            window->draw(shape);
        }

        window->display();
    }
}

int main(int argc, char ** argv)
{
    uint32_t nboids;

    std::cout << "Reading config file" << std::endl;
    Config cfg;
    cfg.readFile("config/config.cfg");
    cfg.lookupValue("nboids", nboids);

    float * xpos = (float *) malloc(sizeof(float)*nboids);
    float * ypos = (float *) malloc(sizeof(float)*nboids);
    float * xvel = (float *) malloc(sizeof(float)*nboids);
    float * yvel = (float *) malloc(sizeof(float)*nboids);
    float * next_xpos = (float *) malloc(sizeof(float)*nboids);
    float * next_ypos = (float *) malloc(sizeof(float)*nboids);
    float * next_xvel = (float *) malloc(sizeof(float)*nboids);
    float * next_yvel = (float *) malloc(sizeof(float)*nboids);

    omp_set_num_threads(3);

    //Lancement du thread de rendu
    XInitThreads();
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flocking Simulation");
    window.setFramerateLimit(60);
    window.setActive(false);
    sf::Thread thread(std::bind(&renderLoop, &window, nboids, xpos, ypos, xvel, yvel));
    thread.launch();

    //on garde les dimensions de la fenêtre dans un coin, ca peut être utile
    sf::Vector2u win_size = window.getSize();
    std::cout << "Actual window size is " <<  win_size.x << " "  << win_size.y << std::endl;

    srand(time(NULL)); // :D

    for(uint32_t i = 0; i < nboids; i++)
    {
        next_xpos[i] = (float) (rand() % win_size.x +1);
        next_ypos[i] = (float) (rand() % win_size.y +1);
        xvel[i] = 0.0f;
        yvel[i] = 0.0f;
        next_xvel[i] = 0.0f;
        next_yvel[i] = 0.0f;
        xpos[i] = 0.0f;
        ypos[i] = 0.0f;
    }

    sf::Clock clock;
    sf::Time elapsed = clock.restart();

    int toggle = 1;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Vector2u win_size = window.getSize();
        sf::Vector2i mouse = sf::Mouse::getPosition(window);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            std::cout << "exiting program" << std::endl;
            window.close();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            toggle = !toggle;

        elapsed = clock.restart();

        if(toggle)
            updateBoidsOpenMP(nboids, elapsed.asSeconds(), win_size.x, win_size.y, mouse.x, mouse.y, xpos, ypos, xvel, yvel, next_xpos, next_ypos, next_xvel, next_yvel);
        //std::cout << "mouse: " << localPosition.x << ", " << localPosition.y << std::endl;
    }

    delete[] xpos; 
    delete[] ypos; 
    delete[] xvel; 
    delete[] yvel; 
    delete[] next_xpos; 
    delete[] next_ypos; 
    delete[] next_xvel; 
    delete[] next_yvel; 
    
    return 0;
}

