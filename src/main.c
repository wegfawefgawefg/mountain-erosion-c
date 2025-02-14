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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow(
        "Erosion Simulation - Heightmap (Rotating Camera)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Enable depth testing for proper 3D rendering.
    glEnable(GL_DEPTH_TEST);

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
    mat4_perspective(proj, 45.0f * (3.14159f / 180.0f), (float)WIDTH / HEIGHT, 1.0f, 100.0f);

    // View matrix: translate the world so it appears in front of the camera.
    // Since the camera is at (0,0,0) and looks down -Z, we move the scene to Z = -3.
    float view[16];
    mat4_translate(view, 0.0f, 20.0f, -3.0f);

    // For the model matrix we combine:
    //  - a constant rotation around X to tilt the mesh (~ -34°)
    //  - a dynamic rotation around Y (so we can see the mesh from different angles)
    float modelX[16];
    mat4_rotate_x(modelX, -0.6f); // -0.6 rad ≈ -34°

    Uint32 lastPrint = 0;
    float angleY = 0.0f;

    // Main render loop.
    while (!state.quit)
    {
        process_input(&state);

        // Update dynamic Y rotation based on time.
        float timeSec = SDL_GetTicks() / 1000.0f;
        angleY = timeSec; // 1 radian per second

        // Print the current rotation (in degrees) once per second.
        if (SDL_GetTicks() - lastPrint > 1000)
        {
            printf("Y Rotation: %.2f degrees\n", angleY * (180.0f / 3.14159f));
            lastPrint = SDL_GetTicks();
        }

        float modelY[16];
        mat4_rotate_y(modelY, angleY);

        // Combine the rotations: model = modelY * modelX.
        float model[16];
        mat4_mul(model, modelY, modelX);

        // Compute the final MVP: MVP = proj * view * model.
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
