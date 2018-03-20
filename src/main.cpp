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

	// Our shader program
	std::shared_ptr<Program> shapeProg;
	std::shared_ptr<Program> groundProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> sphereShape;
	shared_ptr<Shape> boxShape;
	shared_ptr<Shape> bunnyShape;
	shared_ptr<Shape> bushShape;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;
    GLuint IndexBufferID;
	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//reference to texture FBO

	bool FirstTime = true;
	bool Moving = false;
	int gMat = 0;

	bool mouseDown = false;

	float phi = 0;
	float theta = 90;
	float prevX;
	float prevY;
	vec3 cameraPos = vec3(0.0, 0.0, 5.0);

	float x = PI/2;
	float y = 0;
	float z = 0;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveForward = false;
	bool moveBackward = false;

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
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
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

	void groundSetUp(const std::string& resourceDirectory)
	{
		groundProg = make_shared<Program>();
		groundProg->setVerbose(true);
		groundProg->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! groundProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		groundProg->addUniform("P");
		groundProg->addUniform("V");
		groundProg->addUniform("M");
		groundProg->addUniform("MatAmb");
		groundProg->addUniform("MatDif");
		groundProg->addUniform("lightPos");
	    groundProg->addUniform("MatSpec");
	    groundProg->addUniform("shine");
		groundProg->addAttribute("vertPos");
		groundProg->addAttribute("vertNor");
	}

	void shapeSetUp(const std::string& resourceDirectory)
	{
		// Initialize the GLSL program.
		shapeProg = make_shared<Program>();
		shapeProg->setVerbose(true);
		shapeProg->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
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

	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// cTheta = 0;
		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		groundSetUp(resourceDirectory);
		shapeSetUp(resourceDirectory);

	 }

	void initGeom(const std::string& resourceDirectory)
	{

		sphereShape = make_shared<Shape>();
		sphereShape->loadMesh(resourceDirectory + "/sphere.obj");
		sphereShape->resize();
		sphereShape->init();

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
	}


	void render()
	{
		auto ViewUser = make_shared<MatrixStack>();
		// DO NOT TOUCH THE CAMERA =================================
	
		x = cos(radians(phi))*cos(radians(theta));
		y = sin(radians(phi));
		z = cos(radians(phi))*sin(radians(theta));

		vec3 forward = vec3(x, y, z);
		vec3 up = vec3(0,1,0);
		vec3 sides = cross(forward, up);

		if(moveForward)
		{
			cameraPos += forward * MOVEMENT_SPEED;
		}
		if(moveBackward)
		{
			cameraPos -= forward * MOVEMENT_SPEED;
		}
		if(moveLeft)
		{
			cameraPos -= sides * MOVEMENT_SPEED;
		}
		if(moveRight)
		{
			cameraPos += sides * MOVEMENT_SPEED;
		}

		ViewUser->pushMatrix();
			ViewUser->loadIdentity();
			ViewUser->pushMatrix();
			ViewUser->lookAt(vec3(cameraPos.x, 1.0, cameraPos.z), forward + vec3(cameraPos.x, 1.0, cameraPos.z), up);

		MatrixStack *userViewPtr = ViewUser.get();
		drawScene(userViewPtr);
		 
	}

	// have booleon in here for whether to draw the quad or not
	void drawScene(MatrixStack* View)
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);


		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width/(float)height;

		// Create the matrix stacks
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();
		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GROUND PROGRAM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
		

		groundProg->bind();
		Program *gProgPtr = groundProg.get();
		glUniformMatrix4fv(groundProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(groundProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(groundProg->getUniform("lightPos"), -500.0, 500.0, 500.0);

		// globl transforms for 'camera' (you will fix this now!)
		Model->pushMatrix();
			Model->loadIdentity();
				Model->pushMatrix();
				Model->scale(vec3(100.0, 1, 100.0));
				Model->translate(vec3(0.0, -2, 0.0));
				SetMaterial(11, gProgPtr);
				glUniformMatrix4fv(groundProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				boxShape->draw(groundProg);
				Model->popMatrix();

		Model->popMatrix();
		Projection->popMatrix();
		groundProg->unbind();



		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ OBJECT PROGRAM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//Draw our scene - two meshes - right now to a texture
		shapeProg->bind();
		Projection->pushMatrix();
		Program *progPtr = shapeProg.get();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(shapeProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(shapeProg->getUniform("lightPos"), -500.0, 500.0, 500.0);
		// globl transforms for 'camera' (you will fix this now!)
		Model->pushMatrix();
			Model->loadIdentity();
 
			float tx, tz, theta = 0;
			for (int i = 0; i < 10; i++)
			{
				tx = (8.f) * sin(theta + glfwGetTime()/2);
				tz = (8.f) * cos(theta + glfwGetTime()/2);
				/* draw left mesh */
				Model->pushMatrix();
				Model->translate(vec3(tx, 0.f, tz));
				Model->rotate(3.14f + theta + sin(glfwGetTime()) + cos(glfwGetTime()), vec3(0, 1, 0));
				// Model->rotate(radians(-90.f), vec3(1, 0, 0));
				SetMaterial(i % 10, progPtr);
				glUniformMatrix4fv(shapeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				bunnyShape->draw(shapeProg);
				Model->popMatrix();
				theta += 6.28f / 10.f;
			}


		Model->popMatrix();
		Projection->popMatrix();
		View->popMatrix();

		shapeProg->unbind();

	
	}

	// helper function to set materials for shading
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

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
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
