#include "../include/boid.h"
#include <omp.h>

void updateBoidsOpenMP(int nboids, float delta_t, float * xpos, float * ypos, float * rot)
{
#pragma omp parallel for
            for(int i = 0; i < nboids; i++)
            {
                rot[i] += delta_t * 60.0f;
            } 
}

