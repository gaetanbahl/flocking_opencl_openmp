#include "../include/boid.h"
#include <omp.h>
#include <math.h>
#include <stdint.h>

#define SEP 0.1f
#define ALIGN 0.1f
#define COH 0.1f

#define SPEED 0.00001f

#define RANGE 40.0f

inline float distance(uint32_t i, uint32_t j, float * x, float * y)
{
    return sqrt((x[i]-x[j])*(x[i]-x[j]) + (y[i]-y[j])*(y[i]-y[j]));
}

inline float norm(float x, float y)
{
    return sqrt(x*x + y*y);
}


void updateBoidsOpenMP(uint32_t nboids, float delta_t, float * xpos, float * ypos, float * xvel, float * yvel,float * rot)
{

#pragma omp parallel for
    for(uint32_t i = 0; i < nboids; i++)
    {

        uint32_t nb_neigh = 0;

        float avg_posx = 0.0f;
        float avg_posy = 0.0f;

        float avg_velx = 0.0f;
        float avg_vely = 0.0f;

        float sepx = 0.0f;
        float sepy = 0.0f;


        //calcul des voisins
        for(uint32_t j = 0; j < nboids; j++)
        {
            float dist = distance(i,j,xpos,ypos);
            if(dist < RANGE)
            {
                if(j==i)
                    continue;

                nb_neigh ++;

                avg_posx += xpos[j];
                avg_posy += ypos[j];

                avg_velx += xvel[j];
                avg_vely += yvel[j];

                sepx += xpos[j] - xpos[i];
                sepy += ypos[j] - ypos[i];
            }
        }

        if(nb_neigh != 0)
        {
            avg_posx /= (float) nb_neigh;
            avg_posy /= (float) nb_neigh; 
            avg_posx = avg_posx - xpos[i] ;
            avg_posy = avg_posy - ypos[i] ;
            float norm_pos = norm(avg_posx, avg_posy);
            avg_posx /= norm_pos;
            avg_posy /= norm_pos;

            avg_velx /= (float) nb_neigh;
            avg_vely /= (float) nb_neigh;
            float norm_vel = norm(avg_velx, avg_vely);
            avg_velx /= norm_vel;
            avg_vely /= norm_vel;

            sepx /= (float) nb_neigh * -1.0f;
            sepy /= (float) nb_neigh * -1.0f;
            float norm_sep  = norm(sepx, sepy);
            sepx /= norm_sep;
            sepy /= norm_sep;

            xvel[i] += SEP * sepx + ALIGN * avg_velx + COH * avg_posx;
            yvel[i] += SEP * sepy + ALIGN * avg_vely + COH * avg_posy;
            float norm_sp = norm(xvel[i], yvel[i]);
            xvel[i] = xvel[i] / norm_sp * SPEED;
            yvel[i] = yvel[i] / norm_sp * SPEED;
        }


        //rot[i] = acos(yvel[i]/xvel[i]);

        xpos[i] += xvel[i];
        ypos[i] += yvel[i];
    } 
}

