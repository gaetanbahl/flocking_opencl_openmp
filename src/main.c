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
#include "../include/common/oclobject.hpp"
#include "../include/common/utils.h"
#include "../include/common/cmdparser.hpp"
#include "../include/common/basic.hpp"

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

void readConfigFile(struct conf_t * conf, struct boids_t * boids)
{
    Config cfg;
    cfg.readFile("config/config.cfg");
    cfg.lookupValue("nboids", conf->nboids);
    cfg.lookupValue("sep", conf->sep);
    cfg.lookupValue("align", conf->align);
    cfg.lookupValue("coh", conf->coh);
    cfg.lookupValue("edge", conf->edge);
    cfg.lookupValue("mouse", conf->mouse);
    cfg.lookupValue("speed", conf->speed);
    cfg.lookupValue("range", conf->range);
    cfg.lookupValue("mouserange", conf->mouserange);
    cfg.lookupValue("numthreads", conf->numthreads);

    boids->xpos = (float *) malloc(sizeof(float)*conf->nboids);
    boids->ypos = (float *) malloc(sizeof(float)*conf->nboids);
    boids->xvel = (float *) malloc(sizeof(float)*conf->nboids);
    boids->yvel = (float *) malloc(sizeof(float)*conf->nboids);
    boids->next_xpos = (float *) malloc(sizeof(float)*conf->nboids);
    boids->next_ypos = (float *) malloc(sizeof(float)*conf->nboids);
    boids->next_xvel = (float *) malloc(sizeof(float)*conf->nboids);
    boids->next_yvel = (float *) malloc(sizeof(float)*conf->nboids);

    return;
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

int main(int argc, const char ** argv)
{

    std::cout << "Reading config file" << std::endl;
    struct conf_t conf;
    struct boids_t boids;
    readConfigFile(&conf,&boids);

    omp_set_num_threads(conf.numthreads);

    CmdParserCommon cmdparser(argc, argv);
    cmdparser.parse();
    if(cmdparser.help.isSet())
        return 0;

    OpenCLBasic oclobjects(
        cmdparser.platform.getValue(),
        cmdparser.device_type.getValue(),
        cmdparser.device.getValue()
    );

    char kernelBuildArgs[20];
    sprintf(kernelBuildArgs, "-DNBOIDS=%d", conf.nboids);
    OpenCLProgramOneKernel executable(oclobjects, L"kernels/BoidsKernels.cl","","Boids",kernelBuildArgs);

    //Lancement du thread de rendu
    XInitThreads();
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flocking Simulation");
    window.setFramerateLimit(60);
    window.setActive(false);
    sf::Thread thread(std::bind(&renderLoop, &window, conf.nboids, boids.xpos, boids.ypos, boids.xvel, boids.yvel));
    thread.launch();

    //on garde les dimensions de la fenêtre dans un coin, ca peut être utile
    sf::Vector2u win_size = window.getSize();
    std::cout << "Actual window size is " <<  win_size.x << " "  << win_size.y << std::endl;

    srand(time(NULL)); // :D

    for(uint32_t i = 0; i < conf.nboids; i++)
    {
        boids.next_xpos[i] = (float) (rand() % win_size.x +1);
        boids.next_ypos[i] = (float) (rand() % win_size.y +1);
        boids.xvel[i] = 0.0f;
        boids.yvel[i] = 0.0f;
        boids.next_xvel[i] = 0.0f;
        boids.next_yvel[i] = 0.0f;
        boids.xpos[i] = 0.0f;
        boids.ypos[i] = 0.0f;
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
            updateBoidsOpenMP(&conf, elapsed.asSeconds(), win_size.x, win_size.y, mouse.x, mouse.y, &boids);
        //std::cout << "mouse: " << localPosition.x << ", " << localPosition.y << std::endl;
    }

    delete[] boids.xpos; 
    delete[] boids.ypos; 
    delete[] boids.xvel; 
    delete[] boids.yvel; 
    delete[] boids.next_xpos; 
    delete[] boids.next_ypos; 
    delete[] boids.next_xvel; 
    delete[] boids.next_yvel; 
    
    return 0;
}

