/*
Caroline Cullen Program 4 with camera control and monitor using framebuffer
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
#define MOVEMENT_SPEED 0.1f

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;
	std::shared_ptr<Program> groundProg;
	std::shared_ptr<Program> snowmanProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> nefShape;
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
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

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

	void snowmanSetUp(const std::string& resourceDirectory)
	{
		snowmanProg = make_shared<Program>();
		snowmanProg->setVerbose(true);
		snowmanProg->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! snowmanProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		snowmanProg->addUniform("P");
		snowmanProg->addUniform("V");
		snowmanProg->addUniform("M");
		snowmanProg->addUniform("MatAmb");
		snowmanProg->addUniform("MatDif");
		snowmanProg->addUniform("lightPos");
	    snowmanProg->addUniform("MatSpec");
	    snowmanProg->addUniform("shine");
		snowmanProg->addAttribute("vertPos");
		snowmanProg->addAttribute("vertNor");
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
		snowmanSetUp(resourceDirectory);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("lightPos");
	    prog->addUniform("MatSpec");
	    prog->addUniform("shine");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");


		//create two frame buffer objects to toggle between
		glGenFramebuffers(2, frameBuf);
		glGenTextures(2, texBuf);
		glGenRenderbuffers(1, &depthBuf);
		createFBO(frameBuf[0], texBuf[0]);

		//set up depth necessary as rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

		//more FBO set up
		GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers);
		createFBO(frameBuf[1], texBuf[1]);

		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(
			resourceDirectory + "/pass_vert.glsl",
			resourceDirectory + "/tex_fragH.glsl");
		if (! texProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		texProg->addUniform("texBuf");
		texProg->addAttribute("vertPos");
		texProg->addUniform("dir");
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
	 }

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize the obj mesh VBOs etc
		nefShape = make_shared<Shape>();
		nefShape->loadMesh(resourceDirectory + "/Nefertiti-10K.obj");
		nefShape->resize();
		nefShape->init();

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


		//Initialize the geometry to render a quad to the screen
		// ============================Cylinder====================================
        //glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		//generate the VAO

		// ~~~~~~~~~~~~~~~~~~ FOR SNOWMAN ~~~~~~~~~~~~~~~~~~~~~~
        int sides = 20;
        
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);

		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        
        static GLfloat g_vertex_buffer_data[240];
        static GLuint g_index_buffer_data[240];

        int j = 0;
        int i = 0;

        for(; i < sides*3; i+=3)
        {
            g_vertex_buffer_data[i] = (float) (0.4 * cos(j * (PI / sides)*2));
            g_vertex_buffer_data[i+1] = (float) (0.4 * sin(j * (PI / sides)*2));           
            g_vertex_buffer_data[i+2] = (float) (0.1);
            j++;
        } 
        
        j=0;
        for(i = sides*3; i < sides*6; i+=3)
        {
            g_vertex_buffer_data[i] = (float) (0.4 * cos(j * (PI / sides)*2));
            g_vertex_buffer_data[i+1] = (float) (0.4 * sin(j * (PI / sides)*2));
            g_vertex_buffer_data[i+2] = (float) (0.9);
			j++;
        } 
        
        for(i = 0; i < sides*6; i+=6)
        {
            g_index_buffer_data[i] = (i/6);
            g_index_buffer_data[i+1] = sides+(i/6);
            g_index_buffer_data[i+2] = sides+(((i/6)+1)%sides);
            g_index_buffer_data[i+3] = (i/6);
            g_index_buffer_data[i+4] = ((i/6)+1)%sides;
            g_index_buffer_data[i+5] = sides+(((i/6)+1)%sides);       
        }         
        
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &IndexBufferID);
       	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

        // set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), g_index_buffer_data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

        // BINDING FOR NORMALS
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindVertexArray(0);


		initQuad();
	}

	/**** geometry set up for a quad *****/
	void initQuad()
	{
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] =
		{
			-1.2f, -1.2f, 0.0f,
			 1.2f, -1.2f, 0.0f,
			-1.2f,  1.2f, 0.0f,
			-1.2f,  1.2f, 0.0f,
			 1.2f, -1.2f, 0.0f,
			 1.2f,  1.2f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	}

	/* Helper function to create the framebuffer object and
		associated texture to write to */
	void createFBO(GLuint& fb, GLuint& tex)
	{
		//initialize FBO
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		//set up framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//set up texture
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error setting up frame buffer - exiting" << endl;
			exit(0);
		}
	}

	// set up view matricies and stuff in here 
	void render()
	{
		bool drawQuad = false;
		auto ViewUser = make_shared<MatrixStack>();
		auto ViewMonitor = make_shared<MatrixStack>();
		// calculate user view matrix
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
			ViewUser->lookAt(cameraPos, forward + cameraPos, up);

		MatrixStack *userViewPtr = ViewUser.get();

		ViewMonitor->pushMatrix();
			ViewMonitor->loadIdentity();
			ViewMonitor->pushMatrix();
			ViewMonitor->lookAt(vec3(0.0, 0.2 , 15.0), -forward, up);

		MatrixStack *monitorViewPtr = ViewMonitor.get();

		// DO NOT TOUCH THE CAMERA =================================
		// draw scene from quad perspective first ie dont draw the quad


		glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
		drawScene(drawQuad, monitorViewPtr);

		// draw scene with quad
		drawQuad = true;
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		drawScene(drawQuad, userViewPtr);
		 
	}

	// have booleon in here for whether to draw the quad or not
	void drawScene(bool drawQuad, MatrixStack* View)
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

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TEXTURE PROGRAM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if(drawQuad)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texBuf[0]);

			// example applying of 'drawing' the FBO texture - change shaders
			texProg->bind();
			glUniform1i(texProg->getUniform("texBuf"), 0);
			glUniform2f(texProg->getUniform("dir"), -1, 0);
			glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
			Model->pushMatrix();
			Model->loadIdentity();
				Model->pushMatrix();
				Model->translate(vec3(0.0, 0.2, 15.0));
				glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(0);
				Model->popMatrix();
			Model->popMatrix();
			texProg->unbind();
		}

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

		// ~~~~~~~~~~~~~~~~~~ SNOWMAN ~~~~~~~~~~~~~~~~~~~~~~~~~
        snowmanProg->bind();
		Program *snowProgPtr = snowmanProg.get();
        Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		glUniformMatrix4fv(snowmanProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(snowmanProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(snowmanProg->getUniform("lightPos"), -500.0, 500.0, 500.0);

		// head ~~~~~~~~~~~~~~~~~~~~~~~
		Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
	        SetMaterial(3, snowProgPtr);
	        Model->translate(vec3(0, 0.55, 0));
	        Model->scale(vec3(0.2, 0.2, 0.2));

	        float val3 = 0;
	        if(((int)(glfwGetTime()*5)%3)==0)
	        {
	            val3 -= 0.1;
	        }
	        else if(((int)(glfwGetTime()*5)%3)==1)
	        {
	            val3 += 0.025;
	        }
	        else
	        {   
	            val3 += 0.075;
	        }
	        Model->translate(vec3(val3, 0, 0));
	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));
	        sphereShape->draw(snowmanProg);
        	Model->popMatrix();
		Model->popMatrix();
        // body ~~~~~~~~~~~~~~~~~~~~~~~
    	Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();

	        Model->translate(vec3(0, 0, 0));
	        Model->translate(vec3(0.05, 0.12, 0));
	        Model->scale(vec3(0.27, 0.27, 0.27));
			SetMaterial(3, snowProgPtr);
	        float val2 = 0;
	        if(((int)(glfwGetTime()*5)%3)==0)
	        {
	            val2 += 0.2;
	        }
	        else if(((int)(glfwGetTime()*5)%3)==1)
	        {
	            val2 -= 0.3;
	        }
	        else
	        {   
	            val2 += 0.1;
	        }
	        Model->translate(vec3(val2, 0, 0));
	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));


	        sphereShape->draw(snowmanProg);

	        Model->popMatrix();
	    Model->popMatrix();
        	// butt ~~~~~~~~~~~~~~~~~~~~~~~~
        Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();

	        Model->translate(vec3(-0.05 , -0.43, 0));
	        Model->scale(vec3(0.35, 0.35, 0.35));
	        SetMaterial(3, snowProgPtr);
	        float val1 = 0;
	        if(((int)(glfwGetTime()*5)%3)==0)
	        {
	            val1 += 0.5;
	        }
	        else if(((int)(glfwGetTime()*5)%3)==1)
	        {
	            val1 += 0.10;
	        }
	        else
	        {   
	            val1 -= 0.15;
	        }
	        Model->translate(vec3(val1, 0, 0));
	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));

	        sphereShape->draw(snowmanProg);
	        Model->popMatrix();
	    Model->popMatrix();
	        // left hand ~~~~~~~~~~~~~~~~~~~~~~~~
	    Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
			SetMaterial(4, snowProgPtr);
	        Model->translate(vec3(0, 0, 0));
	        Model->translate(vec3(-0.42, 0, 0));
	        Model->rotate(PI/7, vec3(0, 0, 1));
	        Model->rotate(-cos(glfwGetTime()*4)/4, vec3(0, 0, 1));
	        Model->translate(vec3(0, 0.20, 0));
	        Model->scale(vec3(0.03, 0.15, 0.03));

	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));

	        sphereShape->draw(snowmanProg);

	        Model->popMatrix();
	    Model->popMatrix();
	        // left elbow ~~~~~~~~~~~~~~~~~~~~~~~~
		Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
	        SetMaterial(4, snowProgPtr);
	        Model->translate(vec3(0, 0, 0));
	        Model->translate(vec3(-0.45, 0, 0));
	        Model->translate(vec3(0, 0.065, 0));
	        Model->scale(vec3(0.03, 0.04, 0.03));
	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));

	        sphereShape->draw(snowmanProg);

	        Model->popMatrix();
	    Model->popMatrix();
	        // right elbow ~~~~~~~~~~~~~~~~~~~~~~~~
		Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
	        SetMaterial(4, snowProgPtr);
	        Model->translate(vec3(0, 0, 0));
	        Model->translate(vec3(0.45, 0, 0));
	        Model->translate(vec3(0, 0.065, 0));
	        Model->scale(vec3(0.03, 0.04, 0.03));

	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));

	        sphereShape->draw(snowmanProg);

	        Model->popMatrix();
	    Model->popMatrix();
	        // right hand ~~~~~~~~~~~~~~~~~~~~~~~~
	    Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
	        SetMaterial(4, snowProgPtr);
	        Model->translate(vec3(0, 0, 0));
	        Model->translate(vec3(0.42, 0, 0));
	        Model->rotate(-PI/6, vec3(0, 0, 1));
	        Model->rotate(cos(glfwGetTime()*4)/4, vec3(0, 0, 1));
	        Model->translate(vec3(0, 0.20, 0));
	        Model->scale(vec3(0.03, 0.15, 0.03));

	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));

	        sphereShape->draw(snowmanProg);

	        Model->popMatrix();
		Model->popMatrix();
	        // left bicep ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
	        SetMaterial(4, snowProgPtr);
	        Model->translate(vec3(0, 0.2, 0));
	        Model->rotate(-PI/11, vec3(0, 0, 1));
	        Model->rotate(PI/2, vec3(0, 1, 0));

	        Model->scale(vec3(0.1, 0.1, 0.51));

	        glBindVertexArray(VertexArrayID);
	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));

	        glDrawElements(GL_TRIANGLES, 480, GL_UNSIGNED_INT, nullptr);

	        glBindVertexArray(0);

	        Model->popMatrix();

	    Model->popMatrix();

	        // right bicep ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	    Model->pushMatrix();
			Model->loadIdentity();
	        Model->pushMatrix();
	        Model->translate(vec3(0.0, 0.2, 0));
	        Model->rotate(PI/11, vec3(0, 0, 1));
	        Model->rotate(-PI/2, vec3(0, 1, 0));
	        Model->scale(vec3(0.1, 0.1, 0.51));
	        SetMaterial(4, snowProgPtr);
	        glBindVertexArray(VertexArrayID);
	        glUniformMatrix4fv(snowmanProg->getUniform("M"), 1, GL_FALSE, glm::value_ptr(Model->topMatrix()));
	        glDrawElements(GL_TRIANGLES, 480, GL_UNSIGNED_INT, 0);
	        glBindVertexArray(0);

	        Model->popMatrix();

	    Model->popMatrix();
	    Projection->popMatrix();
		// View->popMatrix();

	    snowmanProg->unbind();


		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ OBJECT PROGRAM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//Draw our scene - two meshes - right now to a texture
		prog->bind();
		Projection->pushMatrix();
		Program *progPtr = prog.get();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog->getUniform("lightPos"), -500.0, 500.0, 500.0);
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
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				bunnyShape->draw(prog);
				Model->popMatrix();
				theta += 6.28f / 10.f;
			}

			// for (int i = 0; i < 10; i++)
			// {
			// 	tx = (10.f) * sin(theta);
			// 	tz = (10.f) * cos(theta);
			// 	/* draw left mesh */
			// 	Model->pushMatrix();
			// 	Model->translate(vec3(tx, 0.f, tz));
			// 	Model->rotate(3.14f + theta, vec3(0, 1, 0));
			// 	SetMaterial(i % 10, progPtr);
			// 	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
			// 	bunnyShape->draw(prog);
			// 	Model->popMatrix();
			// 	theta += 6.28f / 10.f;
			// }

			int bushVal = -19;
			for (int i = 0; i < 20; i++)
			{
				Model->pushMatrix();
				Model->translate(vec3(bushVal, -0.2f, -25));
				Model->rotate(radians(20.0f), vec3(0, 1, 0));
				SetMaterial(10, progPtr);
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				bushShape->draw(prog);
				Model->popMatrix();
				bushVal += 2;
			}

			bushVal = -25;
			for (int i = 0; i < 26; i++)
			{
				Model->pushMatrix();
				Model->translate(vec3(-19, -0.2f, bushVal));
				Model->rotate(radians(110.0f), vec3(0, 1, 0));
				SetMaterial(10, progPtr);
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				bushShape->draw(prog);
				Model->popMatrix();
				bushVal += 2;
			}

			bushVal = -25;
			for (int i = 0; i < 26; i++)
			{
				Model->pushMatrix();
				Model->translate(vec3(19, -0.2f, bushVal));
				Model->rotate(radians(110.0f), vec3(0, 1, 0));
				SetMaterial(10, progPtr);
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				bushShape->draw(prog);
				Model->popMatrix();
				bushVal += 2;
			}

			bushVal = -19;
			for (int i = 0; i < 20; i++)
			{
				Model->pushMatrix();
				Model->translate(vec3(bushVal, -0.2f, 25));
				Model->rotate(radians(20.0f), vec3(0, 1, 0));
				SetMaterial(10, progPtr);
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				bushShape->draw(prog);
				Model->popMatrix();
				bushVal += 2;
			}


		Model->popMatrix();

		Projection->popMatrix();
		View->popMatrix();

		prog->unbind();

	
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
