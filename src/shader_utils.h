#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>

char *readFile(const char *filename);
GLuint createShader(GLenum type, const char *filename);
GLuint createShaderProgram();

#endif // SHADER_UTILS_H
