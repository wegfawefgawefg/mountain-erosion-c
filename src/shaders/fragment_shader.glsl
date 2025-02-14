#version 330 core
in float height;
out vec4 FragColor;

void main()
{
//     // Normalize height from [-0.5,0.5] to [0,1]
//     float normalizedHeight = (height + 0.5) / 1.0;
//     // Mix from blue (low) to green (high)
//     vec3 color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), normalizedHeight);
//     FragColor = vec4(color, 1.0);
    // OUTPUT GREEN
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);

}
