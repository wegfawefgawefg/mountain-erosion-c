#include "gen.h"
#include <math.h>
#include <stdlib.h>
#include "util.h" // for rand_range()

static float clampf(float v, float minVal, float maxVal)
{
    if (v < minVal)
        return minVal;
    if (v > maxVal)
        return maxVal;
    return v;
}

void initDroplet(struct Droplet *d)
{
    d->x = rand_range(-1.0f, 1.0f);
    d->z = rand_range(-1.0f, 1.0f);
    d->y = 2.0f;
    d->water = 1.0f;
    d->sediment = 0.0f;
    d->speed = 0.0f;
    d->active = true;
    d->trail_count = 0;
    d->stagnant_steps = 0; // initialize counter
}

float getHeight(struct State *state, float x, float z)
{
    float fx = (x + 1.0f) * 0.5f * (GRID_SIZE - 1);
    float fz = (z + 1.0f) * 0.5f * (GRID_SIZE - 1);
    int ix = (int)fx;
    int iz = (int)fz;
    if (ix < 0)
        ix = 0;
    if (ix >= GRID_SIZE)
        ix = GRID_SIZE - 1;
    if (iz < 0)
        iz = 0;
    if (iz >= GRID_SIZE)
        iz = GRID_SIZE - 1;
    return state->grid[ix][iz];
}

void getGradient(struct State *state, float x, float z, float *gradX, float *gradZ)
{
    const float eps = 0.02f; // increased from 0.001f
    float hXplus = getHeight(state, x + eps, z);
    float hXminus = getHeight(state, x - eps, z);
    float hZplus = getHeight(state, x, z + eps);
    float hZminus = getHeight(state, x, z - eps);

    *gradX = (hXplus - hXminus) / (2.0f * eps);
    *gradZ = (hZplus - hZminus) / (2.0f * eps);
}

void modifyHeight(struct State *state, float x, float z, float amount)
{
    float fx = (x + 1.0f) * 0.5f * (GRID_SIZE - 1);
    float fz = (z + 1.0f) * 0.5f * (GRID_SIZE - 1);
    int ix = (int)fx;
    int iz = (int)fz;
    if (ix < 0 || ix >= GRID_SIZE)
        return;
    if (iz < 0 || iz >= GRID_SIZE)
        return;

    state->grid[ix][iz] += amount;
    if (state->grid[ix][iz] < 0.0f)
        state->grid[ix][iz] = 0.0f;
    if (state->grid[ix][iz] > 2.0f)
        state->grid[ix][iz] = 2.0f;
}

// const for horizontal speed
#define HORIZONTAL_SPEED 0.01f
#define FIXED_SLIDE_STEP 0.001f

// Updated updateDroplet: no erosion; droplet simply falls with gravity and a slight horizontal drift.
// Also updates its trail.
void updateDroplet(struct Droplet *d, struct State *state)
{
    // Out-of-bounds check.
    if (d->x < -1.0f || d->x > 1.0f || d->z < -1.0f || d->z > 1.0f)
    {
        initDroplet(d);
        return;
    }

    const float dt = 0.016f; // ~60 FPS timestep
    const float gravity = 9.8f;
    const float threshold = 0.05f; // free-fall threshold
    const float offset = 0.05f;    // droplet offset above mesh

    float terrainY = getHeight(state, d->x, d->z);
    float gx, gz;
    getGradient(state, d->x, d->z, &gx, &gz);
    float gradLen = sqrtf(gx * gx + gz * gz);
    if (gradLen > 1e-6f)
    {
        gx /= gradLen;
        gz /= gradLen;
    }
    else
    {
        gx = gz = 0.0f;
    }

    // Increment stagnant counter if gradient is nearly zero.
    if (gradLen < 0.01f)
        d->stagnant_steps++;
    else
        d->stagnant_steps = 0;

    // If droplet is stuck for too many steps, reset it.
    if (d->stagnant_steps > MAX_STAGNANT_STEPS)
    {
        initDroplet(d);
        return;
    }

    if (d->y > terrainY + threshold)
    {
        // Free-fall.
        d->y -= gravity * dt;
        d->x -= gx * HORIZONTAL_SPEED * dt;
        d->z -= gz * HORIZONTAL_SPEED * dt;
    }
    else
    {
        // Slide along the mesh using a fixed step.
        d->y = terrainY + offset;
        d->x -= gx * FIXED_SLIDE_STEP;
        d->z -= gz * FIXED_SLIDE_STEP;
    }

    // Update trail.
    if (d->trail_count < TRAIL_LENGTH)
    {
        d->trail[d->trail_count][0] = d->x;
        d->trail[d->trail_count][1] = d->y;
        d->trail[d->trail_count][2] = d->z;
        d->trail_count++;
    }
    else
    {
        for (int i = 0; i < TRAIL_LENGTH - 1; i++)
        {
            d->trail[i][0] = d->trail[i + 1][0];
            d->trail[i][1] = d->trail[i + 1][1];
            d->trail[i][2] = d->trail[i + 1][2];
        }
        d->trail[TRAIL_LENGTH - 1][0] = d->x;
        d->trail[TRAIL_LENGTH - 1][1] = d->y;
        d->trail[TRAIL_LENGTH - 1][2] = d->z;
    }

    // Optional: Check if the trail's bounding box is extremely small (indicating a loop/cavity).
    if (d->trail_count == TRAIL_LENGTH)
    {
        float minX = d->trail[0][0], maxX = d->trail[0][0];
        float minY = d->trail[0][1], maxY = d->trail[0][1];
        float minZ = d->trail[0][2], maxZ = d->trail[0][2];
        for (int i = 1; i < TRAIL_LENGTH; i++)
        {
            if (d->trail[i][0] < minX)
                minX = d->trail[i][0];
            if (d->trail[i][0] > maxX)
                maxX = d->trail[i][0];
            if (d->trail[i][1] < minY)
                minY = d->trail[i][1];
            if (d->trail[i][1] > maxY)
                maxY = d->trail[i][1];
            if (d->trail[i][2] < minZ)
                minZ = d->trail[i][2];
            if (d->trail[i][2] > maxZ)
                maxZ = d->trail[i][2];
        }
        float dx = maxX - minX;
        float dy = maxY - minY;
        float dz = maxZ - minZ;
        const float cavityThreshold = 0.005f;
        if (dx < cavityThreshold && dy < cavityThreshold && dz < cavityThreshold)
            initDroplet(d);
    }
}
