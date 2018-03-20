//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"
#include "WindowManager.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

void Particle::load()
{
	// Random initialization
	rebirth(0.0f);
}

// all particles born at the origin
void Particle::rebirth(float t)
{
	charge = randFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	m = 1.0f;
	d = randFloat(0.0f, 0.02f);
	x.x = 0;
	x.y = 0;
	x.z = randFloat(-3.f, -2.f);
	v.x = randFloat(-0.5f, 0.5f);
	v.y = randFloat(0.5f, 4.0f);
	v.z = randFloat(-0.4f, 0.4f);
	lifespan = randFloat(0.5f, 3.f);
	tEnd = t + lifespan;

	scale = randFloat(0.5f, 10.0f);
	color.r = randFloat(0.2126f, 0.9f);
	color.g = randFloat(0.7152f, 0.8f);
	color.b = randFloat(0.0722f, 0.5f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const bool *keyToggles)
{
	if (t > tEnd)
	{
		rebirth(t);

	}

	// very simple update
	v += h * 0.5f * vec3(0.0f,-9.81f, 0.0f);
	x += h * v;

	// x *= -0.98f;
	//  x.y += -0.8f;

	color.a = (tEnd - t) / lifespan;
}
