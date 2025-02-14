#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "state.h"
#include "shader_utils.h"
#include "input.h"
#include "matrix.h"
#include "gen.h"
#include "util.h" // for rand_range()

#define WIDTH 800
#define HEIGHT 600

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to set override color in the shader.
void setOverrideColor(GLuint shader, bool useOverride, float r, float g, float b, float a)
{
    GLint locOverride = glGetUniformLocation(shader, "useOverrideColor");
    GLint locColor = glGetUniformLocation(shader, "overrideColor");
    glUniform1i(locOverride, useOverride ? 1 : 0);
    glUniform4f(locColor, r, g, b, a);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow(
        "Erosion Simulation - Directional Lighting",
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

    // 1) Initialize droplet
    initDroplet(&state.droplet);

    // 2) Set camera orbit parameters
    state.orbit_angle = 0.0f;
    state.dist = 5.0f;
    state.height = 10.0f;

    // 3) Prepare terrain VBO/VAO
    int numCells = (GRID_SIZE - 1) * (GRID_SIZE - 1);
    int vertexCount = numCells * 6;
    float *vertices = malloc(sizeof(float) * vertexCount * 3);
    if (!vertices)
    {
        printf("Failed to allocate vertex memory.\n");
        return 1;
    }
    generateMesh(vertices, &state);

    GLuint terrainVBO, terrainVAO;
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    free(vertices);

    // 4) Prepare droplet VAO/VBO (for the point)
    GLuint dropletVAO, dropletVBO;
    glGenVertexArrays(1, &dropletVAO);
    glBindVertexArray(dropletVAO);
    glGenBuffers(1, &dropletVBO);
    glBindBuffer(GL_ARRAY_BUFFER, dropletVBO);
    float dropletVertex[3] = {0.0f, 0.0f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(dropletVertex), dropletVertex, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 5) Prepare trail VAO/VBO
    GLuint trailVAO, trailVBO;
    glGenVertexArrays(1, &trailVAO);
    glBindVertexArray(trailVAO);
    glGenBuffers(1, &trailVBO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * TRAIL_LENGTH, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 6) Projection and model matrices.
    float proj[16];
    mat4_perspective(proj, 45.0f * (M_PI / 180.0f), (float)WIDTH / HEIGHT, 1.0f, 100.0f);
    float model[16];
    mat4_identity(model);

    // Set static uniforms.
    glUseProgram(shader_program);
    // Pass the model matrix.
    GLint modelLoc = glGetUniformLocation(shader_program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

    // Set directional light: light coming from above and to the right.
    float lightDir[3] = {0.5f, 1.0f, -0.5f};
    float mag = sqrtf(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
    lightDir[0] /= mag;
    lightDir[1] /= mag;
    lightDir[2] /= mag;
    GLint lightDirLoc = glGetUniformLocation(shader_program, "lightDir");
    glUniform3fv(lightDirLoc, 1, lightDir);

    // Set light color (white).
    GLint lightColorLoc = glGetUniformLocation(shader_program, "lightColor");
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

    // Render loop.
    while (!state.quit)
    {
        process_input(&state);

        if (!state.droplet.active)
        {
            initDroplet(&state.droplet);
        }

        updateDroplet(&state.droplet, &state);

        // Re-generate terrain mesh each frame.
        float *dynamicVerts = malloc(sizeof(float) * vertexCount * 3);
        generateMesh(dynamicVerts, &state);
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, dynamicVerts, GL_STATIC_DRAW);
        free(dynamicVerts);

        // Camera setup.
        float eye[3];
        eye[0] = sinf(state.orbit_angle) * state.dist;
        eye[1] = state.height;
        eye[2] = cosf(state.orbit_angle) * state.dist;
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
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, mvp);

        // Draw terrain.
        setOverrideColor(shader_program, false, 0.0f, 0.0f, 0.0f, 1.0f);
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // Draw droplet point.
        float dropletPos[3] = {state.droplet.x, state.droplet.y, state.droplet.z};
        glBindVertexArray(dropletVAO);
        glBindBuffer(GL_ARRAY_BUFFER, dropletVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dropletPos), dropletPos);
        setOverrideColor(shader_program, true, 0.0f, 0.0f, 0.0f, 1.0f);
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, 1);
        setOverrideColor(shader_program, false, 0.0f, 0.0f, 0.0f, 1.0f);

        // Draw droplet trail.
        glBindVertexArray(trailVAO);
        glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * state.droplet.trail_count, state.droplet.trail);
        setOverrideColor(shader_program, true, 0.0f, 0.0f, 0.0f, 1.0f);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_STRIP, 0, state.droplet.trail_count);
        setOverrideColor(shader_program, false, 0.0f, 0.0f, 0.0f, 1.0f);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteVertexArrays(1, &dropletVAO);
    glDeleteBuffers(1, &dropletVBO);
    glDeleteVertexArrays(1, &trailVAO);
    glDeleteBuffers(1, &trailVBO);
    glDeleteProgram(shader_program);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
