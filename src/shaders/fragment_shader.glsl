#version 330 core
in float height;
out vec4 FragColor;

void main()
{
    vec3 color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), height);
    FragColor = vec4(color, 1.0);
}
