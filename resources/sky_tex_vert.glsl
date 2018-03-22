#version  330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPos;

// out float dCo;
out vec2 vTexCoord;
out vec3 fragNor;

void main() {

  // vec3 lightDir = vec3(1, 1, 1);
  vec4 tpos =  M * vec4(vertPos, 1.0);

  fragNor = ( M * vec4(vertNor, 0.0)).xyz;

  /* First model transforms */
  gl_Position = P * V * tpos;


  /* diffuse coefficient for a directional light */
  // dCo = max(dot(fragNor, normalize(lightPos)), 0);
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
}
