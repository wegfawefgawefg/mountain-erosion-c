#version 330 core
flat in float height;  // flat input, no interpolation
out vec4 FragColor;

uniform bool useOverrideColor;
uniform vec4 overrideColor;

void main()
{
    if (useOverrideColor)
        FragColor = overrideColor;
    else {
        vec3 color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), height);
        FragColor = vec4(color, 1.0);
    }
}
