#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec3 finalColor;
out vec3 fragNor;
out vec3 lightVec;
out vec3 halfVec;
out vec3 WPos;

void main()
{
	gl_Position = P * V * M * vertPos;

	fragNor = normalize((M * vec4(normalize(vertNor), 0.0)).xyz);
	lightVec = normalize(lightPos);

	vec3 viewVec = normalize(vec3(0.0, 0.0, 0.0) - (V * M * vertPos).xyz);
	vec3 l = normalize(lightVec - (vertPos).xyz);

	halfVec = normalize(l + viewVec);
	WPos = vec3(M * vertPos);

}