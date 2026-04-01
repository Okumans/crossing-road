#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;

void main()
{
  TexCoords = aTexCoords;
  gl_Position = u_LightSpaceMatrix * u_Model * vec4(aPos, 1.0);
}
