#ifndef _BOID_H_
#define _BOID_H_

#include <stdint.h>

float norm(float x, float y);

void updateBoidsOpenMP(uint32_t nboids, float delta_t, uint32_t xmax, uint32_t ymax, float * xpos, float * ypos, float * xvel, float * yvel, float * next_xpos, float * next_ypos, float * next_xvel, float * next_yvel);

#endif
