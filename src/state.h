#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#define GRID_SIZE 64

struct State
{
    bool quit;
    float grid[GRID_SIZE][GRID_SIZE];

    // Camera orbit parameters
    float orbit_angle; // in radians
    float dist;        // distance from mountain center
    float height;      // camera height (Y)
};

void initializeGrid(struct State *state);
void generateMesh(float *vertices, struct State *state);

#endif // STATE_H
