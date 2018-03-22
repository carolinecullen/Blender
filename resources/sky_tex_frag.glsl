#version 330 core
uniform sampler2D Texture0;
uniform sampler2D Texture1;

uniform vec3 lightPos;

in vec2 vTexCoord;
in vec4 vertex;
// in float dCo;
out vec4 Outcolor;

void main() {

	vec3 V = normalize(vec3(vTexCoord, 1.0));
	vec3 L = normalize(lightPos);

	float dotProdVL = dot(V, L);

	vec4 Kc = texture(Texture0, vTexCoord);
    vec4 Kg = texture(Texture1, vec2((V.y + 1.0) / 2.0, dotProdVL));
 
  	Outcolor = vec4((Kc.rgb - 0.05f) + (Kg.rgb - 0.05f) / 2.0, 0.5);
  
}

