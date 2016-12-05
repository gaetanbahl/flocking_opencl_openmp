__kernel void Boids(__global float* thing) {
    const int i = get_global_id(0);
    thing[i] = NBOIDS; 
    return;
}
