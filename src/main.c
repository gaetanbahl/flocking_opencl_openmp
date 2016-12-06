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

void renderLoop(sf::RenderWindow * window, int nboids, float * xpositions, float * ypositions, float * xvel, float * yvel, int * opencl)
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
    int toggle = 0;

    std::cout << "Reading config file" << std::endl;
    struct conf_t conf;
    struct boids_t boids;
    readConfigFile(&conf,&boids);

    //Lancement du thread de rendu
    std::cout << "Launching render thread" << std::endl;
    XInitThreads();
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flocking Simulation");
    window.setFramerateLimit(60);
    window.setActive(false);
    sf::Thread thread(std::bind(&renderLoop, &window, conf.nboids, boids.xpos, boids.ypos, boids.xvel, boids.yvel, &toggle));
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

    omp_set_num_threads(conf.numthreads);

    /////////// SETUP OPENCL
    std::cout << "Creating OpenCL objects" << std::endl;
    CmdParserCommon cmdparser(argc, argv);
    cmdparser.parse();
    if(cmdparser.help.isSet())
        return 0;

    OpenCLBasic oclobjects(
        cmdparser.platform.getValue(),
        cmdparser.device_type.getValue(),
        cmdparser.device.getValue()
    );

    std::cout << "Compiling OpenCL Kernel" << std::endl;
    char kernelBuildArgs[200];
    sprintf(kernelBuildArgs, "-DNBOIDS=%d -DSEP=%f -DALIGN=%f -DCOH=%f -DEDGE=%f -DMOUSE=%f -DSPEED=%f -DRANGE=%f -DMOUSERANGE=%f", conf.nboids, conf.sep, conf.align, conf.coh, conf.edge, conf.mouse, conf.speed, conf.range, conf.mouserange);
    OpenCLProgramOneKernel executable(oclobjects, L"kernels/BoidsKernels.cl","","Boids",kernelBuildArgs);

    cl_int err = CL_SUCCESS;

    std::cout << "Mapping OpenCL buffers" << std::endl;
    cl_mem cl_xpos =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.xpos,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);
    
    cl_mem cl_ypos =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.ypos,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);

    cl_mem cl_xvel =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.xvel,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);

    cl_mem cl_yvel =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.yvel,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);

    cl_mem cl_next_xpos =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.next_xpos,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);
    
    cl_mem cl_next_ypos =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.next_ypos,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);

    cl_mem cl_next_xvel =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.next_xvel,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);

    cl_mem cl_next_yvel =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
            conf.nboids * sizeof(cl_float),
            boids.next_yvel,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);

    std::cout << "Setting Kernel Arguments" << std::endl;
    err = clSetKernelArg(executable.kernel, 0, sizeof(cl_mem), (void *) &cl_xpos);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 1, sizeof(cl_mem), (void *) &cl_ypos);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 2, sizeof(cl_mem), (void *) &cl_xvel);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 3, sizeof(cl_mem), (void *) &cl_yvel);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 4, sizeof(cl_mem), (void *) &cl_next_xpos);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 5, sizeof(cl_mem), (void *) &cl_next_ypos);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 6, sizeof(cl_mem), (void *) &cl_next_xvel);
    SAMPLE_CHECK_ERRORS(err);
    err = clSetKernelArg(executable.kernel, 7, sizeof(cl_mem), (void *) &cl_next_yvel);
    SAMPLE_CHECK_ERRORS(err);

    cl_event eventcl;

    size_t global_work_size[1] = {(size_t)conf.nboids};

    sf::Clock clock;
    sf::Time elapsed = clock.restart();
    
    int key = false;

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
        {
            if(!key)
            {
                toggle = !toggle;
                if(toggle)
                    std::cout << "using OpenMP" << std::endl;
                else
                    std::cout << "using OpenCL" << std::endl;
            }
            key = true;
        }
        else
        {
            key = false;
        }

        elapsed = clock.restart();

        if(toggle)
        {
            for(uint32_t i = 0; i < conf.nboids; i++)
            {
                boids.xpos[i] = boids.next_xpos[i];
                boids.ypos[i] = boids.next_ypos[i];
                boids.xvel[i] = boids.next_xvel[i];
                boids.yvel[i] = boids.next_yvel[i];
            }
            updateBoidsOpenMP(&conf, conf.nboids * 0.00001f, 800, 600, 0, 0, &boids);
        }
        else 
        {
            clEnqueueCopyBuffer(oclobjects.queue, cl_next_xpos, cl_xpos, 0, 0, conf.nboids * sizeof(float), 0, NULL, NULL);
            clEnqueueCopyBuffer(oclobjects.queue, cl_next_ypos, cl_ypos, 0, 0, conf.nboids * sizeof(float), 0, NULL, NULL);
            clEnqueueCopyBuffer(oclobjects.queue, cl_next_xvel, cl_xvel, 0, 0, conf.nboids * sizeof(float), 0, NULL, NULL);
            clEnqueueCopyBuffer(oclobjects.queue, cl_next_yvel, cl_yvel, 0, 0, conf.nboids * sizeof(float), 0, NULL, NULL);
            clFinish(oclobjects.queue);

            err = clEnqueueNDRangeKernel(oclobjects.queue, executable.kernel,1, NULL, global_work_size, NULL, 0, NULL, NULL);
            SAMPLE_CHECK_ERRORS(err);
            clFinish(oclobjects.queue);

            clEnqueueReadBuffer(oclobjects.queue, cl_xpos, CL_FALSE,0, conf.nboids*sizeof(float), boids.xpos,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_ypos, CL_FALSE,0, conf.nboids*sizeof(float), boids.ypos,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_xvel, CL_FALSE,0, conf.nboids*sizeof(float), boids.xvel,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_yvel, CL_FALSE,0, conf.nboids*sizeof(float), boids.yvel,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_next_xpos, CL_FALSE,0, conf.nboids*sizeof(float), boids.next_xpos,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_next_ypos, CL_FALSE,0, conf.nboids*sizeof(float), boids.next_ypos,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_next_xvel, CL_FALSE,0, conf.nboids*sizeof(float), boids.next_xvel,0,NULL, NULL); 
            clEnqueueReadBuffer(oclobjects.queue, cl_next_yvel, CL_FALSE,0, conf.nboids*sizeof(float), boids.next_yvel,0,NULL, NULL); 
            clFinish(oclobjects.queue);

        }
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

    err=clFinish(oclobjects.queue);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_xpos);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_ypos);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_xvel);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_yvel);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_next_xpos);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_next_ypos);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_next_xvel);
    SAMPLE_CHECK_ERRORS(err);
    err=clReleaseMemObject(cl_next_yvel);
    SAMPLE_CHECK_ERRORS(err);
    
    return 0;
}

