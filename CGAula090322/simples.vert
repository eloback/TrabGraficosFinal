#version 330 core

layout (location = 0) in vec3 entrada_pos;
layout (location = 1) in vec2 entrada_pos_text;
layout (location = 2) in vec3 entrada_normal;

uniform mat4 mM;
uniform mat4 mV;
uniform mat4 mP;

out vec2 pos_text;
out vec3 normal;
out vec3 posicao;

void main()
{
    pos_text = entrada_pos_text;
    normal = entrada_normal;
    posicao = entrada_pos;
    gl_Position = mP * mV * mM * vec4(entrada_pos, 1.0);
} 