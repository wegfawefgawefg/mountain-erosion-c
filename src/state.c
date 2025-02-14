#include "state.h"
#include <math.h>

// Fill the grid with a sine-wave heightmap normalized to [0, 1.0] (now affecting Y).
void initializeGrid(struct State *state)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            float x = (float)i / (GRID_SIZE - 1);
            float z = (float)j / (GRID_SIZE - 1);
            // Scale the sine/cosine to produce heights between 0.0 and 0.1.
            state->grid[i][j] = 0.5f * sinf(x * 3.1415f * 4) * cosf(z * 3.1415f * 4) + 0.5f;
        }
    }
}

// Generate a mesh (two triangles per grid cell) from the heightmap.
void generateMesh(float *vertices, struct State *state)
{
    int vertex = 0;
    for (int i = 0; i < GRID_SIZE - 1; i++)
    {
        for (int j = 0; j < GRID_SIZE - 1; j++)
        {
            float x0 = (float)i / (GRID_SIZE - 1) * 2 - 1;
            float z0 = (float)j / (GRID_SIZE - 1) * 2 - 1;
            float x1 = (float)(i + 1) / (GRID_SIZE - 1) * 2 - 1;
            float z1 = (float)(j + 1) / (GRID_SIZE - 1) * 2 - 1;

            float y00 = state->grid[i][j];
            float y10 = state->grid[i + 1][j];
            float y01 = state->grid[i][j + 1];
            float y11 = state->grid[i + 1][j + 1];

            // First triangle
            vertices[vertex++] = x0;
            vertices[vertex++] = y00; // HEIGHT AFFECTS Y, NOT Z
            vertices[vertex++] = z0;
            vertices[vertex++] = x1;
            vertices[vertex++] = y10;
            vertices[vertex++] = z0;
            vertices[vertex++] = x0;
            vertices[vertex++] = y01;
            vertices[vertex++] = z1;

            // Second triangle
            vertices[vertex++] = x1;
            vertices[vertex++] = y10;
            vertices[vertex++] = z0;
            vertices[vertex++] = x1;
            vertices[vertex++] = y11;
            vertices[vertex++] = z1;
            vertices[vertex++] = x0;
            vertices[vertex++] = y01;
            vertices[vertex++] = z1;
        }
    }
}
