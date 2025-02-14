#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#define GRID_SIZE 64

struct State
{
    bool quit;
    float grid[GRID_SIZE][GRID_SIZE]; // 64x64 grid of height values
};

void initializeGrid(struct State *state);
void generateMesh(float *vertices, struct State *state);

#endif // STATE_H
