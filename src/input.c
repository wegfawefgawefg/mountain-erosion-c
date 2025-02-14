#include "input.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

void process_input(struct State *state)
{
    SDL_Event event;
    const float zoomSpeed = 0.2f;
    const float orbitSpeed = 0.05f;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            state->quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                state->quit = true;
        }
    }

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    // Adjust distance with W/S
    if (keystate[SDL_SCANCODE_W])
    {
        state->dist -= zoomSpeed;
        if (state->dist < 1.0f)
            state->dist = 1.0f;
    }
    if (keystate[SDL_SCANCODE_S])
    {
        state->dist += zoomSpeed;
    }

    // Rotate around mountain with A/D
    if (keystate[SDL_SCANCODE_A])
    {
        state->orbit_angle -= orbitSpeed;
    }
    if (keystate[SDL_SCANCODE_D])
    {
        state->orbit_angle += orbitSpeed;
    }

    // Debug print
    printf("Camera dist: %.2f, orbit angle: %.2f\n", state->dist, state->orbit_angle);
}
