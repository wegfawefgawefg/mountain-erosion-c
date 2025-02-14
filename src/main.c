#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "state.h"
#include "shader_utils.h"
#include "input.h"
#include "matrix.h" // Matrix math helpers

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
        "Erosion Simulation - Camera Simplified",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    GLuint shader_program = createShaderProgram();
    if (shader_program == 0)
    {
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    struct State state;
    state.quit = false;
    initializeGrid(&state);

    // Set camera orbit parameters
    state.orbit_angle = 0.0f;
    state.dist = 5.0f;
    state.height = 1.0f;

    int numCells = (GRID_SIZE - 1) * (GRID_SIZE - 1);
    int vertexCount = numCells * 6; // 6 vertices per cell (2 triangles)
    float *vertices = malloc(sizeof(float) * vertexCount * 3);
    if (!vertices)
    {
        printf("Failed to allocate vertex memory.\n");
        return 1;
    }
    generateMesh(vertices, &state);

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    free(vertices);

    // Use a 45° FOV and typical near/far values.
    float proj[16];
    mat4_perspective(proj, 45.0f * (M_PI / 180.0f), (float)WIDTH / HEIGHT, 1.0f, 100.0f);

    float model[16];
    mat4_identity(model);

    while (!state.quit)
    {
        process_input(&state);

        // Compute camera position from orbit parameters.
        float eye[3];
        eye[0] = sinf(state.orbit_angle) * state.dist;
        eye[1] = state.height;
        eye[2] = cosf(state.orbit_angle) * state.dist;

        // Always look at the center of the mountain.
        float target[3] = {0.0f, 0.5f, 0.0f};
        float up[3] = {0.0f, 1.0f, 0.0f};

        float view[16];
        mat4_lookAt(view, eye, target, up);

        float pv[16];
        mat4_mul(pv, proj, view);
        float mvp[16];
        mat4_mul(mvp, pv, model);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);
        GLint mvp_location = glGetUniformLocation(shader_program, "mvp");
        // Use GL_FALSE since our matrices are in column-major order.
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, mvp);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
