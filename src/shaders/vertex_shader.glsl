#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 mvp;

out float height;  // Output Y for height-based coloring

void main()
{
    height = aPos.y;  // Use Y instead of Z for coloring
    gl_Position = mvp * vec4(aPos, 1.0);
}
