#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse_0;

void main()
{
  FragColor = texture(texture_diffuse_0, TexCoords);
}
