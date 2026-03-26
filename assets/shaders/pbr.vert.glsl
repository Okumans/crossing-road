#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out mat3 TBN;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(u_Model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_Model))) * aNormal;

    vec3 T = normalize(vec3(u_Model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(u_Model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(u_Model * vec4(aNormal, 0.0)));
    TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * vec4(WorldPos, 1.0);
}
