#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "state.h"
#include "shader_utils.h"
#include "input.h"
#include "matrix.h" // matrix math helpers

#define WIDTH 800
#define HEIGHT 600

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow(
        "Erosion Simulation - Heightmap (Static Grid, Movable Camera)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL);

    // Enable relative mouse mode for mouse look.
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);
    // WIREFRAME MODE
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint shader_program = createShaderProgram();
    if (shader_program == 0)
    {
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Set up the state with a sine-wave heightmap.
    struct State state;
    state.quit = false;
    initializeGrid(&state);

    // Start the camera further back and above the grid.
    state.camX = 0.0f;
    state.camY = 0.0f;
    state.camZ = 0.0f;
    // // Set orientation to look down towards the grid center.
    // // For example, yaw = -45° and pitch = -45°.
    // state.yaw = -45.0f * (M_PI / 180.0f);
    // state.pitch = -45.0f * (M_PI / 180.0f);

    // LOOK STRAIGHT AHEAD
    state.yaw = 0.0f;
    state.pitch = 0.0f;

    // Generate mesh vertices from the grid.
    int numCells = (GRID_SIZE - 1) * (GRID_SIZE - 1);
    int vertexCount = numCells * 6; // 6 vertices per cell (2 triangles)
    float *vertices = malloc(sizeof(float) * vertexCount * 3);
    if (!vertices)
    {
        printf("Failed to allocate vertex memory.\n");
        return 1;
    }
    generateMesh(vertices, &state);

    // Set up VAO/VBO.
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    free(vertices);

    // Projection matrix: 45° fov, proper aspect ratio, near/far planes.
    float proj[16];
    mat4_perspective(proj, 45.0f * (M_PI / 180.0f), (float)WIDTH / HEIGHT, 0.1, 100.0f);

    // Use an identity model matrix (grid is static).
    float model[16];
    mat4_identity(model);

    // Main render loop.
    while (!state.quit)
    {
        process_input(&state);

        // Compute camera forward vector from yaw and pitch.
        float forward[3] = {
            cosf(state.pitch) * sinf(state.yaw),
            sinf(state.pitch),
            cosf(state.pitch) * cosf(state.yaw)};

        // Camera target = position + forward.
        float target[3] = {
            state.camX + forward[0],
            state.camY + forward[1],
            state.camZ + forward[2]};

        // Use world up as (0,1,0).
        float up[3] = {0.0f, 1.0f, 1.0f};

        // Compute view matrix using lookAt.
        float view[16];
        mat4_lookAt(view, (float[]){state.camX, state.camY, state.camZ},
                    target,
                    up);

        // Compute final MVP: MVP = proj * view * model.
        float pv[16];
        mat4_mul(pv, proj, view);
        float mvp[16];
        mat4_mul(mvp, pv, model);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);
        GLint mvp_location = glGetUniformLocation(shader_program, "mvp");
        glUniformMatrix4fv(mvp_location, 1, GL_TRUE, mvp);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        SDL_GL_SwapWindow(window);
    }

    // Cleanup.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
