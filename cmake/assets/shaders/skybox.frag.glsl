#version 450 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;

void main()
{    
    vec3 color = texture(u_Skybox, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
