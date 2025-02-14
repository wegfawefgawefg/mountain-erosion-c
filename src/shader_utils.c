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
