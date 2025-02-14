#include "state.h"
#include <math.h>

// Fill the grid with a sine-wave heightmap normalized to [0,0.1].
void initializeGrid(struct State *state)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            float x = (float)i / (GRID_SIZE - 1);
            float y = (float)j / (GRID_SIZE - 1);
            // Scale the sine/cosine to produce heights between 0.0 and 0.1.
            // state->grid[i][j] = 0.05f * sinf(x * 3.1415f * 4) * cosf(y * 3.1415f * 4) + 0.05f;
            // SET ALL THE HEIGHTS TO 0.0 FOR DEBUG
            state->grid[i][j] = 0.0f;
        }
    }
}

// Generate a mesh (two triangles per grid cell) from the heightmap.
// The grid vertices are already centered in x and y [-1,1].
void generateMesh(float *vertices, struct State *state)
{
    int vertex = 0;
    for (int i = 0; i < GRID_SIZE - 1; i++)
    {
        for (int j = 0; j < GRID_SIZE - 1; j++)
        {
            float x0 = (float)i / (GRID_SIZE - 1) * 2 - 1;
            float y0 = (float)j / (GRID_SIZE - 1) * 2 - 1;
            float x1 = (float)(i + 1) / (GRID_SIZE - 1) * 2 - 1;
            float y1 = (float)(j + 1) / (GRID_SIZE - 1) * 2 - 1;

            float z00 = state->grid[i][j];
            float z10 = state->grid[i + 1][j];
            float z01 = state->grid[i][j + 1];
            float z11 = state->grid[i + 1][j + 1];

            // First triangle
            vertices[vertex++] = x0;
            vertices[vertex++] = y0;
            vertices[vertex++] = z00;
            vertices[vertex++] = x1;
            vertices[vertex++] = y0;
            vertices[vertex++] = z10;
            vertices[vertex++] = x0;
            vertices[vertex++] = y1;
            vertices[vertex++] = z01;

            // Second triangle
            vertices[vertex++] = x1;
            vertices[vertex++] = y0;
            vertices[vertex++] = z10;
            vertices[vertex++] = x1;
            vertices[vertex++] = y1;
            vertices[vertex++] = z11;
            vertices[vertex++] = x0;
            vertices[vertex++] = y1;
            vertices[vertex++] = z01;
        }
    }
}
