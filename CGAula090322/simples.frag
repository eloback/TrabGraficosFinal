#version 330 core

uniform sampler2D textura;
uniform sampler2D textura_normal;

uniform float luz_int_amb;
uniform float luz_int_dif;
uniform vec3 cor_luz;
uniform vec3 pos_luz;

out vec4 corFinal;

in vec2 pos_text;
in vec3 normal;
in vec3 posicao;

void main (void){

	vec3 n = texture(textura_normal, pos_text).xyz;
	n.y = 1 - n.y;
	n = n * 2 - 1;
	
	vec3 norm = normalize(n); 
	vec3 dirLuz = normalize(pos_luz - posicao); 

	float diff = max(dot(norm, dirLuz), 0.0); 

	vec3 resultado_luz_dif = diff * luz_int_dif * cor_luz;

	vec3 resultado_luz = luz_int_amb * cor_luz + resultado_luz_dif;
	
	corFinal = texture(textura, pos_text) * vec4(resultado_luz, 1.0);
}