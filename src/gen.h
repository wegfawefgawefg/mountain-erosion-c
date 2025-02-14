#ifndef GEN_H
#define GEN_H

#include "state.h"

// Spawn or reset a droplet above the terrain.
void initDroplet(struct Droplet *d);

// Update droplet movement/erosion for one step.
void updateDroplet(struct Droplet *d, struct State *state);

// Helper: get terrain height at floating coords (x,z).
float getHeight(struct State *state, float x, float z);

// Helper: get partial derivatives at (x,z).
void getGradient(struct State *state, float x, float z, float *gradX, float *gradZ);

// Helper: modify height around (x,z).
void modifyHeight(struct State *state, float x, float z, float amount);

#endif // GEN_H
