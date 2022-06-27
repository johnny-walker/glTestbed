#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out VS_OUT {
    out vec3 WorldPos; 
    out vec3 Normal;
    out vec2 TexCoords;
} vs_out;

void main()
{
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;    
    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}