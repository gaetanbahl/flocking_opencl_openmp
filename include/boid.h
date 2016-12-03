#ifndef _BOID_H_
#define _BOID_H_

#include <stdint.h>

struct conf_t {
    uint32_t nboids;
    uint32_t numthreads;
    float sep;
    float align;
    float coh;
    float edge;
    float mouse;
    float speed;
    float range;
    float mouserange;
};

float norm(float x, float y);

void updateBoidsOpenMP(struct conf_t * conf, float delta_t, uint32_t xmax, uint32_t ymax, int32_t mousex, int32_t mousey, float * xpos, float * ypos, float * xvel, float * yvel, float * next_xpos, float * next_ypos, float * next_xvel, float * next_yvel);

#endif
