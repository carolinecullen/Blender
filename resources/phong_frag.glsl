#version 330 core 

//to send the color to a frame buffer
layout(location = 0) out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform vec3 MatDif;
uniform float shine;

out vec4 outputColor;

in vec3 finalColor;
in vec3 fragNor;
in vec3 lightVec;
in vec3 halfVec;
in vec3 WPos;

/* Very simple Diffuse shader with a directional light*/
void main()
{

	vec3 k_a = MatAmb;
	vec3 k_s = MatSpec;
	vec3 k_d = MatDif;

	// for spectral lighting
	float phong = clamp((dot(normalize(fragNor), lightVec)), 0, 1);
	float specPhong = pow((clamp((dot(halfVec, normalize(fragNor))), 0, 1)), shine);

	vec3 phongColor = k_a + (k_d * phong) + (k_s * specPhong);

	outputColor = vec4(phongColor, 1.0);
}
