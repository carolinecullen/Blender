#version  330 core
layout(location = 0) in vec4 vertPos;

out vec2 texCoord;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main()
{
    gl_Position = P * V * M * vertPos;
	texCoord = (vertPos.xy+vec2(1, 1))/2.0;
}
