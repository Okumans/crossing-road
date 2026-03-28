#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_DiffuseTex0;
uniform sampler2D u_HeightTex0;
uniform vec3 u_BaseColor0;

void main()
{
  FragColor = texture(u_DiffuseTex0, TexCoords) * vec4(u_BaseColor0, 1.0);
}
