/*
Caroline Cullen 
Blender
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"
#include "Particle.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.1415
#define MOVEMENT_SPEED 0.2f

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
	int width, height;
	// programs
	shared_ptr<Program> shapeProg;
	shared_ptr<Program> groundProg;
	shared_ptr<Program> particleProg;
	shared_ptr<Program> skyProg;

	// textures 

	shared_ptr<Texture> skyTexture;
	shared_ptr<Texture> groundTexture;
	shared_ptr<Texture> particleTexture;

	//ground info
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int gGiboLen;

	//transforms for the world
	vec3 worldTrans = vec3(0);
	float worldScale = 1.0;

	// Shape to be used (from obj file)
	shared_ptr<Shape> sphereShape;
	shared_ptr<Shape> boxShape;
	shared_ptr<Shape> bunnyShape;
	shared_ptr<Shape> bushShape;
	shared_ptr<Shape> Blender;
	shared_ptr<Shape> fallTree;
	shared_ptr<Shape> deadTree;
	shared_ptr<Shape> bean;


	// Contains vertex information for OpenGL
	GLuint GroundVertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//stuff necessary for particles
	vector<std::shared_ptr<Particle>> particles;
	GLuint ParticleVertexArrayID;
	int numP = 600;
	GLfloat points[1800];
	GLfloat pointColors[2400];
	GLuint particlePointsBuffer;
	GLuint particleColorBuffer;
	float t0_disp = 0.0f;
	float t_disp = 0.0f;
	bool keyToggles[256] = { false };
	float t = 0.0f;
	float h = 0.01f;
	glm::vec3 g = glm::vec3(0.0f, -0.01f, 0.0f);


	bool FirstTime = true;
	bool sprint = false;
	bool Moving = false;
	int gMat = 0;

	bool mouseDown = false;

	float phi = 0;
	float theta = 90;
	float prevX;
	float prevY;
	vec3 cameraPos = vec3(0.0, 0.0, 5.0);
	vec3 lightPos = vec3(500.0, 500.0, 500.0);

	float x = PI/2;
	float y = 0;
	float z = 0;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveForward = false;
	bool moveBackward = false;

	int numTrees;
	vector<GLfloat> treePositions;
	vector<GLfloat> treeScales;
	vector<GLfloat> treeRotations;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			moveLeft = true;

		}
		else if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			moveRight = true;

		}
		else if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			moveForward = true;

		}
		else if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			moveBackward = true;

		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			moveLeft = false;

		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			moveRight = false;

		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			moveForward = false;

		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			moveBackward = false;

		}
		else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		{
			sprint = true;

		}
		else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		{
			sprint = false;

		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		// cTheta += (float) deltaX;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			Moving = true;
			prevX = posX;
			prevY = posY;
		}

		if (action == GLFW_RELEASE)
		{
			Moving = false;
			mouseDown = false;
			prevX = 0;
			prevY = 0;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if(mouseDown == true)
		{
			phi -=(float)(prevY - ypos);
			prevY = ypos;
			
			if(phi > 80)
			{
				phi = 80;
			}
			else if(phi < -80)
			{
				phi = -80;
			}
			
			theta +=(float) (prevX - xpos);
			prevX = xpos;

		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	// Code to load in textures
	void initTex(const std::string& resourceDirectory)
	{
		skyTexture = make_shared<Texture>();
		skyTexture->setFilename(resourceDirectory + "/sky.jpg");
		skyTexture->init();
		skyTexture->setUnit(0);
		skyTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	 	groundTexture = make_shared<Texture>();
		groundTexture->setFilename(resourceDirectory + "/ground.jpg");
		groundTexture->init();
		groundTexture->setUnit(1);
		groundTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

		particleTexture = make_shared<Texture>();
		particleTexture->setFilename(resourceDirectory + "/alpha.bmp");
		particleTexture->init();
		particleTexture->setUnit(2);
		particleTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);	
	}

	void groundSetUp(const std::string& resourceDirectory)
	{
		//initialize the textures we might use
		groundProg = make_shared<Program>();
		groundProg->setVerbose(true);
		groundProg->setShaderNames(
			resourceDirectory + "/tex_vert.glsl",
			resourceDirectory + "/tex_frag.glsl");
		if (! groundProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		groundProg->addUniform("P");
		groundProg->addUniform("V");
		groundProg->addUniform("M");
		groundProg->addUniform("Texture0");
		groundProg->addUniform("texNum");
		groundProg->addAttribute("vertPos");
		groundProg->addAttribute("vertNor");
		groundProg->addAttribute("vertTex");
		groundProg->addUniform("lightPos");
	}

	void shapeSetUp(const std::string& resourceDirectory)
	{
		// Initialize the GLSL program.
		shapeProg = make_shared<Program>();
		shapeProg->setVerbose(true);
		shapeProg->setShaderNames(
			resourceDirectory + "/phong_vert.glsl",
			resourceDirectory + "/phong_frag.glsl");
		if (! shapeProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		shapeProg->addUniform("P");
		shapeProg->addUniform("V");
		shapeProg->addUniform("M");
		shapeProg->addUniform("MatAmb");
		shapeProg->addUniform("MatDif");
		shapeProg->addUniform("lightPos");
	    shapeProg->addUniform("MatSpec");
	    shapeProg->addUniform("shine");
		shapeProg->addAttribute("vertPos");
		shapeProg->addAttribute("vertNor");
	}

	void particleSetUp(const std::string& resourceDirectory)
	{
		particleProg = make_shared<Program>();
		particleProg->setVerbose(true);
		particleProg->setShaderNames(
			resourceDirectory + "/particle_vert.glsl",
			resourceDirectory + "/particle_frag.glsl");
		if (! particleProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		particleProg->addUniform("P");
		particleProg->addUniform("V");
		particleProg->addUniform("M");
		particleProg->addUniform("alphaTexture");
		particleProg->addAttribute("vertPos");
	}

	void skySetUp(const std::string& resourceDirectory)
	{
		skyProg = make_shared<Program>();
		skyProg->setVerbose(true);
		skyProg->setShaderNames(
			resourceDirectory + "/sky_tex_vert.glsl",
			resourceDirectory + "/sky_tex_frag.glsl");
		if (! skyProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		skyProg->addUniform("P");
		skyProg->addUniform("V");
		skyProg->addUniform("M");
		skyProg->addUniform("Texture0");
		skyProg->addUniform("texNum");
		skyProg->addUniform("lightPos");
		skyProg->addAttribute("vertPos");
		skyProg->addAttribute("vertNor");
		skyProg->addAttribute("vertTex");
	}

	void init(const std::string& resourceDirectory)
	{
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		glClearColor(.12f, .34f, .56f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		
		skySetUp(resourceDirectory);
		shapeSetUp(resourceDirectory);
		groundSetUp(resourceDirectory);
		particleSetUp(resourceDirectory);
		
	 }

	void initParticles()
	{
		int n = numP;

		for (int i = 0; i < n; ++ i)
		{
			auto particle = make_shared<Particle>();
			particles.push_back(particle);
			particle->load();
		}
	}

	void initTrees()
	{
		numTrees = randGen(300.f, 500.f);
		
		for(int i = 0; i < numTrees; i++)
		{
			treePositions.push_back(randGen(-512.f, 512.f));
			treePositions.push_back(randGen(-512.f, 512.f));
			treeScales.push_back(randGen(3.0f, 7.0f));
			treeRotations.push_back(randGen(0.0f, 180.0f));
		}
	}

	void initGeom(const std::string& resourceDirectory)
	{

		boxShape = make_shared<Shape>();
		boxShape->loadMesh(resourceDirectory + "/cube.obj");
		boxShape->resize();
		boxShape->init();

		bunnyShape = make_shared<Shape>();
		bunnyShape->loadMesh(resourceDirectory + "/bunny.obj");
		bunnyShape->resize();
		bunnyShape->init();

		bushShape = make_shared<Shape>();
		bushShape->loadMesh(resourceDirectory + "/bush.obj");
		bushShape->resize();
		bushShape->init();


		// need to make it so i can read i multiple shapes like the dummy 
		// also new shader that doesnt expect the vertTex attribute
		Blender = make_shared<Shape>();
		Blender->loadMesh(resourceDirectory + "/blender.obj");
		Blender->resize();
		Blender->init();

		fallTree = make_shared<Shape>();
		fallTree->loadMesh(resourceDirectory + "/fallTree.obj");
		fallTree->resize();
		fallTree->init();

		deadTree = make_shared<Shape>();
		deadTree->loadMesh(resourceDirectory + "/deadTree.obj");
		deadTree->resize();
		deadTree->init();

		bean = make_shared<Shape>();
		bean->loadMesh(resourceDirectory + "/beanstalk.obj");
		bean->resize();
		bean->init();

		// for ground
		initQuad();

		// creation for particles
		CHECKED_GL_CALL(glGenVertexArrays(1, &ParticleVertexArrayID));
		CHECKED_GL_CALL(glBindVertexArray(ParticleVertexArrayID));

		CHECKED_GL_CALL(glGenBuffers(1, &particlePointsBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particlePointsBuffer));

		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glGenBuffers(1, &particleColorBuffer));
		
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
		// creation for particles

		sphereShape = make_shared<Shape>();
		sphereShape->loadMesh(resourceDirectory + "/sphere.obj");
		sphereShape->resize();
		sphereShape->init();
	}

	void updateGeom()
	{
		glm::vec3 pos;
		glm::vec4 col;

		for (int i = 0; i < numP; i++)
		{
			pos = particles[i]->getPosition();
			col = particles[i]->getColor();
			points[i * 3 + 0] = pos.x;
			points[i * 3 + 1] = pos.y;
			points[i * 3 + 2] = pos.z;
			pointColors[i * 4 + 0] = col.r + col.a / 10.f;
			pointColors[i * 4 + 1] = col.g + col.g / 10.f;
			pointColors[i * 4 + 2] = col.b + col.b / 10.f;
			pointColors[i * 4 + 3] = col.a;
		}

		// update the GPU data
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particlePointsBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void updateParticles()
	{
		// update the particles
		for (auto particle : particles)
		{
			particle->update(t, h, g, keyToggles);
		}
		t += h;

		// Sort the particles by Z
		auto temp = make_shared<MatrixStack>();
		temp->rotate(y, vec3(0, 1, 0));

		ParticleSorter sorter;
		sorter.C = temp->topMatrix();
		std::sort(particles.begin(), particles.end(), sorter);
	}

	void initQuad()
	{
		float g_groundSize = 512;
		float g_groundY = -1.5;

		// A x-z plane at y = g_groundY of dim[-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			 g_groundSize, g_groundY,  g_groundSize,
			 g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		float GrndTex[] = {
			0, 0, // back
			0, 1,
			1, 1,
			1, 0
		};

		unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		GLuint GroundVertexArrayID;
		//generate the VAO
		glGenVertexArrays(1, &GroundVertexArrayID);
		glBindVertexArray(GroundVertexArrayID);

		gGiboLen = 6;
		glGenBuffers(1, &GrndBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndNorBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndTexBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

		glGenBuffers(1, &GIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
	}

	void render()
	{

		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width/(float)height;

		x = cos(radians(phi))*cos(radians(theta));
		y = sin(radians(phi));
		z = cos(radians(phi))*sin(radians(theta));

		vec3 forward = vec3(x, y, z);
		vec3 up = vec3(0,1,0);
		vec3 sides = cross(forward, up);

		float actualSpeed = MOVEMENT_SPEED;
		if(sprint)
		{
			actualSpeed *= 2;
		}

		if(moveForward)
		{
			cameraPos += forward * actualSpeed;

		}
		if(moveBackward)
		{

			cameraPos -= forward * actualSpeed;

		}
		if(moveLeft)
		{

			cameraPos -= sides * actualSpeed;

		}
		if(moveRight)
		{

			cameraPos += sides * actualSpeed;

		}

		auto ViewUser = make_shared<MatrixStack>();

		ViewUser->pushMatrix();
			ViewUser->loadIdentity();
			ViewUser->pushMatrix();
			ViewUser->lookAt(vec3(cameraPos.x, 1.0, cameraPos.z), forward + vec3(cameraPos.x, 1.0, cameraPos.z), up);
		MatrixStack *userViewPtr = ViewUser.get();

		auto Projection = make_shared<MatrixStack>();
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);
		MatrixStack *projectionPtr = Projection.get();


		CHECKED_GL_CALL(glDisable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glDisable(GL_BLEND));
		drawSky(userViewPtr, projectionPtr);

		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		drawScene(userViewPtr, projectionPtr);
		drawGround(userViewPtr, projectionPtr);

		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(25.0f));
		drawParticles(userViewPtr, projectionPtr);

		Projection->popMatrix();
		ViewUser->popMatrix();
		ViewUser->popMatrix();	 
	}

	void drawParticles(MatrixStack* View, MatrixStack* Projection)
	{
		particleProg->bind();
		updateParticles();
		updateGeom();

		auto Model = make_shared<MatrixStack>();
		Model->pushMatrix();
			Model->loadIdentity();

		particleTexture->bind(particleProg->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(particleProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(particleProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(particleProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix())));

		CHECKED_GL_CALL(glEnableVertexAttribArray(0));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particlePointsBuffer));
		CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0));

		CHECKED_GL_CALL(glEnableVertexAttribArray(1));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer));
		CHECKED_GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0));

		CHECKED_GL_CALL(glVertexAttribDivisor(0, 1));
		CHECKED_GL_CALL(glVertexAttribDivisor(1, 1));
		// Draw the points !
		CHECKED_GL_CALL(glDrawArraysInstanced(GL_POINTS, 0, 1, numP));

		CHECKED_GL_CALL(glVertexAttribDivisor(0, 0));
		CHECKED_GL_CALL(glVertexAttribDivisor(1, 0));
		CHECKED_GL_CALL(glDisableVertexAttribArray(0));
		CHECKED_GL_CALL(glDisableVertexAttribArray(1));
		particleTexture->unbind();

		Model->popMatrix();
		particleProg->unbind();
	}

	void renderGround()
	{

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glDrawElements(GL_TRIANGLES, gGiboLen, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	void drawGround(MatrixStack* View, MatrixStack* Projection)
	{
		auto Model = make_shared<MatrixStack>();
		groundProg->bind();
		glUniformMatrix4fv(groundProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(groundProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(groundProg->getUniform("lightPos"), 500.0, 500.0, 500.0);

		Model->pushMatrix();
			Model->loadIdentity();
				Model->pushMatrix();
				glUniformMatrix4fv(groundProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()) );
				groundTexture->bind(groundProg->getUniform("Texture0"));
				glUniform1f(groundProg->getUniform("texNum"), 500);
				renderGround();
				Model->popMatrix();	
				groundTexture->unbind();


		Model->popMatrix();
		groundProg->unbind();
	}

	void drawSky(MatrixStack* View, MatrixStack* Projection)
	{
		auto Model = make_shared<MatrixStack>();
		skyProg->bind();
		glUniformMatrix4fv(skyProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(skyProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(skyProg->getUniform("lightPos"), 500.0, 500.0, 500.0);
		Model->pushMatrix();
			Model->loadIdentity();
				Model->pushMatrix();
				Model->translate(cameraPos);
				Model->scale(vec3(50, 50.f, 50));
				glUniformMatrix4fv(skyProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()) );
				skyTexture->bind(skyProg->getUniform("Texture0"));
				glUniform1f(skyProg->getUniform("texNum"), 1);
				sphereShape->draw(skyProg);
				Model->popMatrix();	
				skyTexture->unbind();


		Model->popMatrix();
		skyProg->unbind();
	}

	void drawScene(MatrixStack* View, MatrixStack* Projection)
	{

		auto Model = make_shared<MatrixStack>();
		Program *sProgPtr = shapeProg.get();

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ OBJECT PROGRAM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		shapeProg->bind();

		glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(shapeProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(shapeProg->getUniform("lightPos"), 500.0, 500.0, 500.0);

		Model->pushMatrix();
			Model->loadIdentity();
 
			for (int i = 0; i < numTrees; i+=3)
			{
				/* draw left mesh */
				GLfloat treeS = treeScales[i/3];
				GLfloat treeR = treeRotations[i/3];
				Model->pushMatrix();
				Model->translate(vec3(treePositions[i], treeS/1.60, treePositions[i+2]));
				Model->scale(vec3(treeS));
				Model->rotate(treeR, vec3(0, 1, 0));
				SetMaterial(i % 10, sProgPtr);
				glUniformMatrix4fv(shapeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				fallTree->draw(shapeProg);
				Model->popMatrix();
				
			}


		Model->popMatrix();
		shapeProg->unbind();
	}

	void SetMaterial(int i, Program *prog)
	{
		switch (i)
		{
		case 0: // shiny blue plastic
		    glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
		    glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
		    glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
		    glUniform1f(prog->getUniform("shine"), 120.0);
		    break;
	    case 1: // flat grey
		    glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
		    glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
		    glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
		    glUniform1f(prog->getUniform("shine"), 4.0);
		    break;
	    case 2: // brass
		    glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
		    glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
		    glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
		    glUniform1f(prog->getUniform("shine"), 27.9);
		    break;
        case 3: // pearl
	        glUniform3f(prog->getUniform("MatAmb"), 0.25f, 0.20725f, 0.20725f);
	        glUniform3f(prog->getUniform("MatDif"), 1.0f, 0.829f, 0.829f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.296648f, 0.296648f, 0.296648f);
	        glUniform1f(prog->getUniform("shine"), 11.264f);
	        break;
	    case 4: // copper
	        glUniform3f(prog->getUniform("MatAmb"),  0.19125f, 0.0735f, 0.0225f);
	        glUniform3f(prog->getUniform("MatDif"), 0.7038f, 0.27048f, 0.0828f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.256777f, 0.137622f, 0.086014f);
	        glUniform1f(prog->getUniform("shine"), 12.8f);
	        break;
		case 5: // turqoise
	        glUniform3f(prog->getUniform("MatAmb"),  0.1f, 0.18725f, 0.1745f);
	        glUniform3f(prog->getUniform("MatDif"), 0.396f, 0.74151f, 0.69102f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.297254f, 0.30829f, 0.306678f);
	        glUniform1f(prog->getUniform("shine"), 12.8f);
	        break;
	    case 6: // obisdian
	        glUniform3f(prog->getUniform("MatAmb"),  0.05375f, 0.05f, 0.06625f);
	        glUniform3f(prog->getUniform("MatDif"), 0.18275f, 0.17f, 0.22525f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.332741f, 0.328634f, 0.346435f);
	        glUniform1f(prog->getUniform("shine"), 38.4f);
	        break;
	    case 7: // ruby
	        glUniform3f(prog->getUniform("MatAmb"),  0.1745f, 0.01175f, 0.01175f);
	        glUniform3f(prog->getUniform("MatDif"), 0.61424f, 0.04136f, 0.04136f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.727811f, 0.626959f, 0.626959f);
	        glUniform1f(prog->getUniform("shine"), 76.8f);
	        break;
	    case 8: // emerald
	        glUniform3f(prog->getUniform("MatAmb"),  0.0215f, 0.1745f, 0.0215f);
	        glUniform3f(prog->getUniform("MatDif"), 0.07568f, 0.61424f, 0.07568f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.633f, 0.727811f, 0.633f);
	        glUniform1f(prog->getUniform("shine"), 76.8f);
	        break;
	    case 9: // yellow plastic
	        glUniform3f(prog->getUniform("MatAmb"),  0.05f,0.05f,0.0f);
	        glUniform3f(prog->getUniform("MatDif"), 0.5f,0.5f,0.4f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.7f,0.7f,0.04f);
	        glUniform1f(prog->getUniform("shine"), 10.0f);
	        break;

	    case 10: // bush
	        glUniform3f(prog->getUniform("MatAmb"),  0.0215f, 0.1745f, 0.0215f);
	        glUniform3f(prog->getUniform("MatDif"), 0.07568f, 0.61424f, 0.07568f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.633f, 0.727811f, 0.633f);
	        glUniform1f(prog->getUniform("shine"), 30.8f);
	        break;

	    case 11: // ground
	        glUniform3f(prog->getUniform("MatAmb"),  0.09f,0.54f,0.13f);
	        glUniform3f(prog->getUniform("MatDif"), 0.0f,0.0f,0.0f);
	        glUniform3f(prog->getUniform("MatSpec"), 0.0f,0.0f,0.0f);
	        glUniform1f(prog->getUniform("shine"), 1.0f);
	        break;
		}
	}

	float randGen(float l, float h)
	{
		float r = rand() / (float) RAND_MAX;
		return (1.0f - r) * l + r * h;
	}

};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
			resourceDir = argv[1];
	}

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initParticles();
	application->initTrees();
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
			// Render scene.
			application->render();

			// Swap front and back buffers.
			glfwSwapBuffers(windowManager->getHandle());

			// Poll for and process events.
			glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
