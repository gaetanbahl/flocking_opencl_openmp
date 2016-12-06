
inline float distance_tab(unsigned int i, unsigned int j,__global float * x,__global float * y)
{
    return sqrt((x[i]-x[j])*(x[i]-x[j]) + (y[i]-y[j])*(y[i]-y[j]));
}

float norm(float x, float y)
{
    return sqrt(x*x + y*y);
}

inline void normalise( float * x, float * y)
{
    if( *x == 0 && *y == 0)
        return;

    float n = norm(*x,*y);
    *x /= n;
    *y /= n;
}

inline void normalise_global(__global float * x,__global float * y)
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

__kernel void Boids(
    __global float* xpos,
    __global float* ypos,
    __global float* xvel,
    __global float* yvel,
    __global float* next_xpos,
    __global float* next_ypos,
    __global float* next_xvel,
    __global float* next_yvel
) {
    const int i = get_global_id(0);
    xpos[i] = NBOIDS; 

    const float sep = 0.13f;
    const float align = 0.105f;
    const float coh = 0.1f;
    const float edge = 0.1f;
    const float mouse = 0.5f;
    const float speed = 50.0f;
    const float range = 40.0f;
    const float mouserange = 80.0f;
    const delta_t = 0.1f;
    int xmax = 800;
    int ymax = 600;
    int mousex = 0;
    int mousey = 0;

    unsigned int nb_neigh = 0;

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

    for(unsigned int j = 0; j < NBOIDS ; j++)
    {
        float dist = distance_tab(i,j,xpos,ypos);
        if(dist < range)
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
        
        if(xpos[i] < range)
            edgex = range - xpos[i];
        else if(xpos[i] > xmax - range)
            edgex = xmax - range - xpos[i];

        if(ypos[i] < range)
            edgey = range - ypos[i];
        else if(ypos[i] > ymax - range)
            edgey = ymax - range - ypos[i];

        if(mousex > 0 && mousex < (int) xmax && mousey > 0 && mousey < (int) ymax && (distancexy(mousex, mousey, xpos[i],ypos[i]) < mouserange))
        {
            mx = mousex - xpos[i];
            my = mousey - ypos[i];
        }
    }

    if(nb_neigh > 0)
    {
        avg_posx /= (float) nb_neigh;
        avg_posy /= (float) nb_neigh; 
        avg_posx = avg_posx - xpos[i] ;
        avg_posy = avg_posy - ypos[i] ;
        normalise(&avg_posx,&avg_posy);

        avg_velx /= (float) nb_neigh;
        avg_vely /= (float) nb_neigh;
        normalise(&avg_velx,&avg_vely);

        sepx /= (float) nb_neigh * -1.0f;
        sepy /= (float) nb_neigh * -1.0f;
        normalise(&sepx, &sepy);

        
    }

    normalise(&edgex, &edgey);
    mx *= -1.0f;
    my *= -1.0f;
    normalise(&mx, &my);

    next_xvel[i] = xvel[i] + mouse * mx + edge * edgex +  sep * sepx + align * avg_velx + coh * avg_posx;
    next_yvel[i] = yvel[i] + mouse * my + edge * edgey +  sep * sepy + align * avg_vely + coh * avg_posy;
    normalise_global(next_xvel + i, next_yvel + i);

    next_xpos[i] = xpos[i] + delta_t * speed * next_xvel[i];
    next_ypos[i] = ypos[i] + delta_t * speed * next_yvel[i];


    return;
}
