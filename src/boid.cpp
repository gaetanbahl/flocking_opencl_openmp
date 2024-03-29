#include "../include/boid.h"
#include <omp.h>
#include <math.h>
#include <stdint.h>
#include <iostream>
#include "../include/common/oclobject.hpp"
#include "../include/common/utils.h"
#include "../include/common/cmdparser.hpp"
#include "../include/common/basic.hpp"

inline float distance(uint32_t i, uint32_t j, float * x, float * y)
{
    return sqrt((x[i]-x[j])*(x[i]-x[j]) + (y[i]-y[j])*(y[i]-y[j]));
}

float norm(float x, float y)
{
    return sqrt(x*x + y*y);
}

inline void normalize(float * x, float * y)
{
    if( *x == 0 && *y == 0)
        return;

    float n = norm(*x,*y);
    *x /= n;
    *y /= n;
}

inline float distancexy(float x1, float y1, float x2, float y2)
{
    return sqrt((x1-x2)*(x1-x2) +(y1-y2)*(y1-y2));
}


void updateBoidsOpenMP(struct conf_t * conf, float delta_t, uint32_t xmax, uint32_t ymax, int32_t mousex, int32_t mousey, struct boids_t * boids)
{
    for(uint32_t i = 0; i < conf->nboids; i++)
    {
        boids->xvel[i] = boids->next_xvel[i];
        boids->yvel[i] = boids->next_yvel[i];
        boids->xpos[i] = boids->next_xpos[i];
        boids->ypos[i] = boids->next_ypos[i];
    }

#pragma omp parallel for
    for(uint32_t i = 0; i < conf->nboids; i++)
    {

        uint32_t nb_neigh = 0;

        float avg_posx = 0.0f;
        float avg_posy = 0.0f;

        float avg_velx = 0.0f;
        float avg_vely = 0.0f;

        float sepx = 0.0f;
        float sepy = 0.0f;

        float edgex = 0.0f;
        float edgey = 0.0f;

        float mx = 0.0f;
        float my = 0.0f;

        for(uint32_t j = 0; j < conf->nboids; j++)
        {
            float dist = distance(i,j,boids->xpos,boids->ypos);
            if(dist < conf->range)
            {
                if(j==i)
                    continue;

                nb_neigh ++;

                avg_posx += boids->xpos[j];
                avg_posy += boids->ypos[j];

                avg_velx += boids->xvel[j];
                avg_vely += boids->yvel[j];

                sepx += boids->xpos[j] - boids->xpos[i];
                sepy += boids->ypos[j] - boids->ypos[i];
            }
            
            if(boids->xpos[i] < conf->range)
                edgex = conf->range - boids->xpos[i];
            else if(boids->xpos[i] > xmax - conf->range)
                edgex = xmax - conf->range - boids->xpos[i];

            if(boids->ypos[i] < conf->range)
                edgey = conf->range - boids->ypos[i];
            else if(boids->ypos[i] > ymax - conf->range)
                edgey = ymax - conf->range - boids->ypos[i];

            if(mousex > 0 && mousex < (int32_t) xmax && mousey > 0 && mousey < (int32_t) ymax && (distancexy(mousex, mousey, boids->xpos[i],boids->ypos[i]) < conf->mouserange))
            {
                mx = mousex - boids->xpos[i];
                my = mousey - boids->ypos[i];
            }
        }

        if(nb_neigh > 0)
        {
            avg_posx /= (float) nb_neigh;
            avg_posy /= (float) nb_neigh; 
            avg_posx = avg_posx - boids->xpos[i] ;
            avg_posy = avg_posy - boids->ypos[i] ;
            normalize(&avg_posx,&avg_posy);

            avg_velx /= (float) nb_neigh;
            avg_vely /= (float) nb_neigh;
            normalize(&avg_velx,&avg_vely);

            sepx /= (float) nb_neigh * -1.0f;
            sepy /= (float) nb_neigh * -1.0f;
            normalize(&sepx, &sepy);

            
        }

        normalize(&edgex, &edgey);
        mx *= -1.0f;
        my *= -1.0f;
        normalize(&mx, &my);

        boids->next_xvel[i] = boids->xvel[i] + conf->mouse * mx + conf->edge * edgex +  conf->sep * sepx + conf->align * avg_velx + conf->coh * avg_posx;
        boids->next_yvel[i] = boids->yvel[i] + conf->mouse * my + conf->edge * edgey +  conf->sep * sepy + conf->align * avg_vely + conf->coh * avg_posy;
        normalize(boids->next_xvel + i, boids->next_yvel + i);

        boids->next_xpos[i] = boids->xpos[i] + delta_t * conf->speed * boids->next_xvel[i];
        boids->next_ypos[i] = boids->ypos[i] + delta_t * conf->speed * boids->next_yvel[i];

    } 
}

