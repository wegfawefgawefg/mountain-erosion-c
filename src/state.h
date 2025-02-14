#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#define GRID_SIZE 64
#define TRAIL_LENGTH 32 // number of positions to store in the trail

#define TRAIL_LENGTH 32      // number of positions to store in the trail
#define MAX_STAGNANT_STEPS 8 // for example, 60 frames (~1 sec at 60 FPS)

struct Droplet
{
    float x, y, z;  // x,z horizontal; y vertical position
    float sediment; // unused for now
    float water;    // unused for now
    float speed;    // unused for now
    bool active;    // is droplet alive/active?

    // Trail: store last TRAIL_LENGTH positions (each as x,y,z)
    float trail[TRAIL_LENGTH][3];
    int trail_count; // number of valid positions in the trail

    int stagnant_steps; // number of consecutive steps with near-zero gradient
};

struct State
{
    bool quit;
    float grid[GRID_SIZE][GRID_SIZE];

    // Camera orbit parameters
    float orbit_angle; // in radians
    float dist;        // distance from mountain center
    float height;      // camera height (Y)

    // We'll keep one droplet for demonstration
    struct Droplet droplet;
};

// from state.c
void initializeGrid(struct State *state);
void generateMesh(float *vertices, struct State *state);

#endif // STATE_H
