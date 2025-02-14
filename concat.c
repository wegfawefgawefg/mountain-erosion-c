///// Directory structure:
./src
├── gen.c
├── gen.h
├── input.c
├── input.h
├── main.c
├── matrix.h
├── shaders
│   ├── fragment_shader.glsl
│   └── vertex_shader.glsl
├── shader_utils.c
├── shader_utils.h
├── state.c
├── state.h
├── util.c
└── util.h

1 directory, 14 files

///// filename: ./src/gen.c

///// filename: ./src/gen.h

///// filename: ./src/input.c
#include "input.h"
#include <SDL2/SDL.h>

void process_input(struct State *state)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            state->quit = true;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                state->quit = true;
            }
            break;
        }
    }
}

///// filename: ./src/input.h
#ifndef INPUT_H
#define INPUT_H

#include "state.h"

void process_input(struct State *state);

#endif // INPUT_H
///// filename: ./src/main.c
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

///// filename: ./src/matrix.h
#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

// Set the 4x4 matrix 'm' to the identity matrix.
static void mat4_identity(float *m)
{
    for (int i = 0; i < 16; i++)
        m[i] = 0.0f;
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

// Create a perspective projection matrix.
// fov: field of view in radians, aspect: width/height,
// near and far: clipping planes.
static void mat4_perspective(float *m, float fov, float aspect, float near, float far)
{
    float f = 1.0f / tanf(fov * 0.5f);
    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}

// Multiply two 4x4 matrices: result = a * b.
static void mat4_mul(float *result, const float *a, const float *b)
{
    float temp[16];
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            temp[row * 4 + col] = 0.0f;
            for (int i = 0; i < 4; i++)
            {
                temp[row * 4 + col] += a[row * 4 + i] * b[i * 4 + col];
            }
        }
    }
    for (int i = 0; i < 16; i++)
        result[i] = temp[i];
}

// Create a rotation matrix around the X axis.
static void mat4_rotate_x(float *m, float angle)
{
    mat4_identity(m);
    m[5] = cosf(angle);
    m[6] = -sinf(angle);
    m[9] = sinf(angle);
    m[10] = cosf(angle);
}

// Create a rotation matrix around the Y axis.
static void mat4_rotate_y(float *m, float angle)
{
    mat4_identity(m);
    m[0] = cosf(angle);
    m[2] = sinf(angle);
    m[8] = -sinf(angle);
    m[10] = cosf(angle);
}

// Create a translation matrix.
static void mat4_translate(float *m, float x, float y, float z)
{
    mat4_identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

#endif // MATRIX_H

///// filename: ./src/shaders/fragment_shader.glsl
#version 330 core
in float height;
out vec4 FragColor;

void main()
{
    // Normalize height from [-0.5,0.5] to [0,1]
    float normalizedHeight = (height + 0.5) / 1.0;
    // Mix from blue (low) to green (high)
    vec3 color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), normalizedHeight);
    FragColor = vec4(color, 1.0);
}

///// filename: ./src/shaders/vertex_shader.glsl
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 mvp;

out float height;

void main()
{
    height = aPos.z;
    gl_Position = mvp * vec4(aPos, 1.0);
}

///// filename: ./src/shader_utils.c
#include "shader_utils.h"
#include <stdio.h>
#include <stdlib.h>

char *readFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    if (buffer == NULL)
    {
        printf("Failed to allocate memory for file: %s\n", filename);
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

GLuint createShader(GLenum type, const char *filename)
{
    char *source = readFile(filename);
    if (source == NULL)
    {
        return 0;
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar **)&source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation failed:\n%s\n", infoLog);
        glDeleteShader(shader);
        free(source);
        return 0;
    }

    free(source);
    return shader;
}

GLuint createShaderProgram()
{
    char *shader_dir = "src/shaders/";
    char *vertex_shader_path = "vertex_shader.glsl";
    char *fragment_shader_path = "fragment_shader.glsl";
    char vertex_shader_full_path[256];
    char fragment_shader_full_path[256];
    sprintf(vertex_shader_full_path, "%s%s", shader_dir, vertex_shader_path);
    sprintf(fragment_shader_full_path, "%s%s", shader_dir, fragment_shader_path);

    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertex_shader_full_path);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragment_shader_full_path);
    if (vertexShader == 0 || fragmentShader == 0)
    {
        return 0;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking failed:\n%s\n", infoLog);
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

///// filename: ./src/shader_utils.h
#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>

char *readFile(const char *filename);
GLuint createShader(GLenum type, const char *filename);
GLuint createShaderProgram();

#endif // SHADER_UTILS_H

///// filename: ./src/state.c
#include "state.h"
#include <math.h>

// Fill the grid with a sine-wave heightmap.
void initializeGrid(struct State *state)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            float x = (float)i / (GRID_SIZE - 1);
            float y = (float)j / (GRID_SIZE - 1);
            // A sine and cosine combination gives a wavy heightmap.
            state->grid[i][j] = 0.5f * sinf(x * 3.1415f * 4) * cosf(y * 3.1415f * 4) + 0.5f;
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
            // Convert grid indices to clip-space positions in x and y.
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

///// filename: ./src/state.h
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

///// filename: ./src/util.c
#include "util.h"
#include <stdlib.h>

float rand_range(float min, float max)
{
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

///// filename: ./src/util.h

#ifndef UTIL_H
#define UTIL_H

#include "util.h"

float rand_range(float min, float max);

#endif

