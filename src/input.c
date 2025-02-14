#include "input.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper: update camera vectors based on yaw and pitch.
static void getCameraVectors(const struct State *state, float *forward, float *right)
{
    // Forward vector.
    forward[0] = cosf(state->pitch) * sinf(state->yaw);
    forward[1] = sinf(state->pitch);
    forward[2] = cosf(state->pitch) * cosf(state->yaw);
    // Right vector: perpendicular to forward and world up (0,1,0).
    right[0] = sinf(state->yaw - (M_PI / 2));
    right[1] = 0.0f;
    right[2] = cosf(state->yaw - (M_PI / 2));
}

void process_input(struct State *state)
{
    SDL_Event event;
    const float moveSpeed = 0.1f;
    const float mouseSensitivity = 0.002f; // Adjust as needed.

    // Process all events.
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
        case SDL_MOUSEMOTION:
        {
            // Adjust yaw and pitch based on relative mouse movement.
            state->yaw += event.motion.xrel * mouseSensitivity;
            state->pitch += event.motion.yrel * mouseSensitivity;
            // Clamp pitch to avoid flipping (e.g., between -89 and +89 degrees).
            if (state->pitch > (89.0f * M_PI / 180.0f))
                state->pitch = 89.0f * M_PI / 180.0f;
            if (state->pitch < (-89.0f * M_PI / 180.0f))
                state->pitch = -89.0f * M_PI / 180.0f;
            break;
        }
        }
    }

    // Get the current state of the keyboard for continuous movement.
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    float forward[3], right[3];
    getCameraVectors(state, forward, right);

    // Movement: W/S for forward/backward, A/D for left/right,
    // SPACE for up, Left Shift for down.
    if (keystate[SDL_SCANCODE_W])
    {
        state->camX += forward[0] * moveSpeed;
        state->camY += forward[1] * moveSpeed;
        state->camZ += forward[2] * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_S])
    {
        state->camX -= forward[0] * moveSpeed;
        state->camY -= forward[1] * moveSpeed;
        state->camZ -= forward[2] * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_A])
    {
        state->camX -= right[0] * moveSpeed;
        state->camZ -= right[2] * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_D])
    {
        state->camX += right[0] * moveSpeed;
        state->camZ += right[2] * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_SPACE])
    {
        state->camY += moveSpeed;
    }
    if (keystate[SDL_SCANCODE_LSHIFT])
    {
        state->camY -= moveSpeed;
    }

    // Print camera position and angles.
    printf("Camera Pos: (%.2f, %.2f, %.2f) | Yaw: %.2f deg, Pitch: %.2f deg\n",
           state->camX, state->camY, state->camZ,
           state->yaw * 180.0f / M_PI, state->pitch * 180.0f / M_PI);
}
