#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

void presentation();

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::mat4 modelRotation;
glm::mat4 normalMatrixRotation;

//matrices for the pinwheel
glm::mat4 modelPinwheel;

// shader uniform locations
GLint modelLoc;
GLint modelPinwheelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;

GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPosEyeLoc;

GLuint fogLoc;
GLuint transparencyLoc;

GLuint shadowMapFBO;
GLuint depthMapTexture;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;
float sensitivityRotateCamera = 0.3f;

float angleY = 0.0f;
GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
GLfloat angle;
gps::Model3D parkScene;
gps::Model3D felinare;
gps::Model3D house;
gps::Model3D windows;
gps::Model3D pinwheel_stick;
gps::Model3D pinwheel_petals;

// shaders
gps::Shader myBasicShader;
gps::Shader mySkyBoxShader;
gps::Shader myDepthMapShader;

//fog variables
float fogDensityValue;

//transparency variables;
float transparencyLevel = 0.4f;
float noTransparency = 1.0f;

//skybox
gps::SkyBox mySkyBox;

//mouse variables
bool pressed = false;
bool mouse = true;
float lastX = 400, lastY = 300;
float yaw = -90.0f;
float pitch;

//lights
float lightIntensity;

//shadows
glm::vec3 lightDir = glm::vec3(5.0f, 30.0f, 9.0f);
glm::vec3 lightCentre = glm::vec3(0.0f, 0.0f, 0.0f);
glm::mat4 lightRotation;
GLfloat lightAngle = 0;
glm::vec3 lightColor;
glm::vec4 lightPosEye;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) 
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{

		if (!pressed)
		{
			lastX = xpos;
			lastY = ypos;
			pressed = true;
		}
		else
		{

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

			lastX = xpos;
			lastY = ypos;

			xoffset *= sensitivityRotateCamera;
			yoffset *= sensitivityRotateCamera;

			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			myCamera.rotate(pitch, yaw);

			//get view matrix for current camera
			view = myCamera.getViewMatrix();
			myBasicShader.useShaderProgram();
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			//compute normal matrix for teapot
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		pressed = false;
	}
}


void processMovement() {
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	} 

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	} 

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	} 

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	} 

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	} 

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	} 

	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	} 

	if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	} 

	if (pressedKeys[GLFW_KEY_X])
	{
		fogDensityValue = 0.05f;
	} 

	if (pressedKeys[GLFW_KEY_C])
	{
		fogDensityValue = 0.0f;
	} 

	if (pressedKeys[GLFW_KEY_G]) {
		if (lightIntensity < 1.0f)
		{
			lightIntensity += 0.01f;
			lightColor = glm::vec3(lightIntensity, lightIntensity, lightIntensity);			
		}
		lightPosEye.z += 0.05f;
		std::cout << lightPosEye.z << "\n";
	} 
	if (pressedKeys[GLFW_KEY_H]) {
		if (lightIntensity > 0.0f)
		{
			lightIntensity -= 0.01f;
			lightColor = glm::vec3(lightIntensity, lightIntensity, lightIntensity);
		}
		lightPosEye.z -= 0.05f;
		std::cout << lightPosEye.z << "\n";
	} 

	if (pressedKeys[GLFW_KEY_U]) {
		std::cout << "Cam Pos x: " << myCamera.cameraPosition.x << std::endl;
		std::cout << "Cam Pos y: " << myCamera.cameraPosition.y << std::endl;
		std::cout << "Cam Pos z: " << myCamera.cameraPosition.z << std::endl;
		std::cout << "Cam Tar x: " << myCamera.cameraTarget.x << std::endl;
		std::cout << "Cam Tar y: " << myCamera.cameraTarget.y << std::endl;
		std::cout << "Cam Tar z: " << myCamera.cameraTarget.z << std::endl;
		//yaw pitch
		std::cout << "Yaw:       " << yaw << std::endl;
		std::cout << "pitch:     " << pitch << std::endl;
		std::cout << "fogdensit: " << fogDensityValue << std::endl;
	} 

	if (pressedKeys[GLFW_KEY_P]) {
		presentation();
	}
	
	//NORMAL
	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//WIREFRAME
	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//POINT
	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Park Project");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	//For transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void initModels() {
    //teapot.LoadModel("models/teapots/teapot_moved.obj");
    parkScene.LoadModel("objects/test1/park2.obj");
	house.LoadModel("objects/test1/house2.obj");
	windows.LoadModel("objects/test1/windows1.obj");
	pinwheel_stick.LoadModel("objects/test1/pinwheel/pinwheel_stick_final.obj");
	pinwheel_petals.LoadModel("objects/test1/pinwheel/pinwheel_test1.obj");

	//initialize skybox
	std::vector<const GLchar*> faces;
	faces.push_back("textures/skybox/negx.jpg");  //right
	faces.push_back("textures/skybox/posx.jpg");  //left
	faces.push_back("textures/skybox/posy.jpg");  //top
	faces.push_back("textures/skybox/negy.jpg");  //bottom
	faces.push_back("textures/skybox/negz.jpg");  //back
	faces.push_back("textures/skybox/posz.jpg");  //front
	mySkyBox.Load(faces);
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

	myDepthMapShader.loadShader(
		"shaders/depthMap.vert",
		"shaders/depthMap.frag"
	);

	mySkyBoxShader.loadShader(
		"shaders/skyboxShader.vert",
		"shaders/skyboxShader.frag"
	);
}

void initUniforms() {

	myBasicShader.useShaderProgram();

    // create model matrix for teapot
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
	 
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)); glCheckError();
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir)); glCheckError();
	 

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	 
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
	 
	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 100.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	
	 
	//set the light direction (direction towards the light)
	//lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
	 
	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	 
	//fog
	fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
	glUniform1f(fogLoc, fogDensityValue);
	 
	//transparency
	transparencyLoc = glGetUniformLocation(myBasicShader.shaderProgram, "transparency");
	glUniform1f(transparencyLoc, transparencyLevel);
	 
	//skybox
	mySkyBoxShader.useShaderProgram();
	viewLoc = glGetUniformLocation(mySkyBoxShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	projectionLoc = glGetUniformLocation(mySkyBoxShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	 
}

void renderHouse(gps::Shader shader)
{
	shader.useShaderProgram();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	house.Draw(shader);
}

void renderWindows(gps::Shader shader)
{
	shader.useShaderProgram();
	glUniform1f(transparencyLoc, transparencyLevel);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	windows.Draw(shader);

	glUniform1f(transparencyLoc, noTransparency);
}

void renderParkScene(gps::Shader shader)
{
    // select active shader program
    shader.useShaderProgram();
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	glUniform1f(fogLoc, fogDensityValue); //call the uniform that creates the fog

    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    parkScene.Draw(shader);
}

float pinWheelRotationAngle = 0.0f;
void renderPinWheel(gps::Shader shader)
{
	//-------------for the stick----------------------------
	// select active shader program
	shader.useShaderProgram();

	//send pinwheel stick model matrix data to shader

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//send pinwheel stick normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//draw pinwheel stick
	pinwheel_stick.Draw(shader);

	//--------------for the petals now----------------------
	// select active shader program
	shader.useShaderProgram();

	//send pinwheel stick model matrix data to shader
		//create model for pinwheel
	pinWheelRotationAngle = pinWheelRotationAngle - 1;
	modelPinwheel = glm::translate(glm::mat4(1.0f), glm::vec3(-5.51f, 0.58f, -3.38f));
	modelPinwheel = glm::rotate(modelPinwheel, glm::radians(pinWheelRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	modelPinwheel = glm::translate(modelPinwheel, glm::vec3(5.51f, -0.58f, 3.38f));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPinwheel));

	//send pinwheel stick normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//draw pinwheel stick
	pinwheel_petals.Draw(shader);
}

void renderAllObjects(gps::Shader shader) {
	//render the  park scene
	renderParkScene(shader);

	//render the house
	renderHouse(shader);

	//render the pinwheel
	renderPinWheel(shader);

	//render the semi-transparent windows
	renderWindows(shader);

	//create model for pinwheel
	modelPinwheel = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void renderScene() 
{
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)); 
	myBasicShader.useShaderProgram();
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw the skybox
	mySkyBox.Draw(mySkyBoxShader, view, projection);

	//render all the objects needed for the scene
	renderAllObjects(myBasicShader); 
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	//close GL context and any other GLFW resources
	glfwTerminate();
	myWindow.Delete();
	//cleanup code for your own data
}

void presentation() {
	std::vector<glm::vec3> positions;
	positions.push_back(glm::vec3(-5.803f, 0.717f, 15.29f));
	positions.push_back(glm::vec3(-5.251f, 0.982f, 10.734f));
	positions.push_back(glm::vec3(-3.051f, 0.620f, 1.990f));
	positions.push_back(glm::vec3(1.861f, 0.616f, -1.090f));
	positions.push_back(glm::vec3(5.425f, 0.597f, -2.093f));
	positions.push_back(glm::vec3(6.700f, 0.603f, -2.688f));
	positions.push_back(glm::vec3(9.144f, 0.577f, -4.698f));
	positions.push_back(glm::vec3(6.473f, 0.574f, -6.292f));
	positions.push_back(glm::vec3(-1.557f, 0.827f, -4.685f));
	positions.push_back(glm::vec3(-3.706f, 0.822f, -1.156f));
	positions.push_back(glm::vec3(-4.332f, 1.411f, 0.627f));
	positions.push_back(glm::vec3(-9.224f, 1.480f, 3.153f));
	positions.push_back(glm::vec3(-7.788f, 0.792f, -3.278f));
	positions.push_back(glm::vec3(-7.814f, 0.650f, -3.5801f));
	positions.push_back(glm::vec3(-6.365f, 0.716f, -4.407f));
	positions.push_back(glm::vec3(-7.709f, 0.909f, -7.282f));
	positions.push_back(glm::vec3(-6.307f, 0.967f, -13.573f));
	positions.push_back(glm::vec3(-14.072f, 0.839f, -15.874f));
	positions.push_back(glm::vec3(-24.011f, 0.822f, -21.292f));
	positions.push_back(glm::vec3(-32.158f, 0.905f, -18.894f));
	positions.push_back(glm::vec3(-36.851f, 0.974f, -14.810f));
	positions.push_back(glm::vec3(-38.574f, 0.970f, -9.303f));
	positions.push_back(glm::vec3(-40.486f, 0.966f, -1.401f));
	positions.push_back(glm::vec3(-40.561f, 0.986f, 5.915f));
	positions.push_back(glm::vec3(-36.778f, 1.150f, 7.517f));
	positions.push_back(glm::vec3(-36.778f, 1.150f, 7.517f));
	positions.push_back(glm::vec3(-34.137f, 1.327f, 11.054f));
	positions.push_back(glm::vec3(-32.445f, 2.156f, 19.017f));

	//positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); //last and ignored

	std::vector<glm::vec3> targets;
	targets.push_back(glm::vec3(-5.683f, 0.775f, 14.302f));
	targets.push_back(glm::vec3(-5.131f, 1.040f, 9.743f));
	targets.push_back(glm::vec3(-2.395f, 0.584f, 1.237f));
	targets.push_back(glm::vec3(2.825f, 0.611f, -1.366f));
	targets.push_back(glm::vec3(5.851f, 0.602f, -2.997f));
	targets.push_back(glm::vec3(6.679f, 0.555f, -3.686f));
	targets.push_back(glm::vec3(8.301f, 0.624f, -5.233f));
	targets.push_back(glm::vec3(5.492f, 0.511f, -6.105f));
	targets.push_back(glm::vec3(-2.495f, 0.786f, -4.342f));
	targets.push_back(glm::vec3(-4.413f, 0.775f, -1.863f));
	targets.push_back(glm::vec3(-4.863f, 1.275f, -0.209f));
	targets.push_back(glm::vec3(-9.088f, 1.422f, 2.164f));
	targets.push_back(glm::vec3(-7.737f, 0.594f, -4.257f));
	targets.push_back(glm::vec3(-7.737f, 0.806f, -4.564f));
	targets.push_back(glm::vec3(-7.047f, 0.800f, -5.133f));
	targets.push_back(glm::vec3(-8.171f, 1.003f, -8.164f));
	targets.push_back(glm::vec3(-7.266f, 0.951f, -13.857f));
	targets.push_back(glm::vec3(-15.0315f, 0.824f, -16.158f));
	targets.push_back(glm::vec3(-23.880f, 0.801f, -20.301f));
	targets.push_back(glm::vec3(-31.212f, 0.884f, -18.570f));
	targets.push_back(glm::vec3(-35.861f, 0.980f, -14.664f));
	targets.push_back(glm::vec3(-37.585f, 0.975f, -9.157f));
	targets.push_back(glm::vec3(-39.585f, 1.007f, -1.322f));
	targets.push_back(glm::vec3(-39.565f, 1.028f, 5.993f));
	targets.push_back(glm::vec3(-35.815f, 1.165f, 7.248f));
	targets.push_back(glm::vec3(-36.440f, 1.092f, 6.577f));
	targets.push_back(glm::vec3(-34.210f, 1.222f, 10.062f));
	targets.push_back(glm::vec3(-32.259f, 2.061f, 18.039f));

	//targets.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); //last and ignored

	std::vector<float> yaws;
	yaws.push_back(-83.1f);
	yaws.push_back(-83.1f);
	yaws.push_back(-48.9f);
	yaws.push_back(-15.6f);
	yaws.push_back(-64.8f);
	yaws.push_back(-91.2001f);
	yaws.push_back(-147.6f);
	yaws.push_back(-190.8f);
	yaws.push_back(-200.1f);
	yaws.push_back(-135.0f);
	yaws.push_back(-122.4f);
	yaws.push_back(-82.2f);
	yaws.push_back(-86.9999f);
	yaws.push_back(-85.4999f);
	yaws.push_back(-133.2f);
	yaws.push_back(-117.6f);
	yaws.push_back(-163.5f);
	yaws.push_back(-163.5f);
	yaws.push_back(-277.5f);
	yaws.push_back(-341.1f);
	yaws.push_back(-351.6f);
	yaws.push_back(-351.6f);
	yaws.push_back(-355.5f);
	yaws.push_back(-355.5f);
	yaws.push_back(-355.599f);
	yaws.push_back(-430.199f);
	yaws.push_back(-454.199f);
	yaws.push_back(-439.199f);

	//yaws.push_back(0.0f); //last and ignored

	std::vector<float> pitches;
	pitches.push_back(-3.300002f);
	pitches.push_back(-3.300002f);
	pitches.push_back(-2.09998f);
	pitches.push_back(-0.299983f);
	pitches.push_back(-0.300017f);
	pitches.push_back(-2.69998f);
	pitches.push_back(2.7f);
	pitches.push_back(-3.59998f);
	pitches.push_back(-2.39998f);
	pitches.push_back(-2.7f);
	pitches.push_back(-7.8f);
	pitches.push_back(-3.3f);
	pitches.push_back(-11.4f);
	pitches.push_back(9.0f);
	pitches.push_back(4.8f);
	pitches.push_back(5.4f);
	pitches.push_back(0.900003f);
	pitches.push_back(0.900003f);
	pitches.push_back(-1.2f);
	pitches.push_back(-1.2f);
	pitches.push_back(0.3f);
	pitches.push_back(0.3f);
	pitches.push_back(2.4f);
	pitches.push_back(2.4f);
	pitches.push_back(0.9f);
	pitches.push_back(-3.3f);
	pitches.push_back(-6.0f);
	pitches.push_back(-5.4f);

	//pitches.push_back(0.0f); //last and ignored

	std::vector<float> fogs;
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.05f);
	fogs.push_back(0.05f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);
	fogs.push_back(0.0f);

	//fogs.push_back(0.0f); //last and ignored

	while (positions.size() > 1) {

		for (float i = 0.0f; i < 1.0f; i += 0.004) {

			glm::vec3 cameraPos;
			cameraPos.x = (1 - i) * positions.at(0).x + i * positions.at(1).x;
			cameraPos.y = (1 - i) * positions.at(0).y + i * positions.at(1).y;
			cameraPos.z = (1 - i) * positions.at(0).z + i * positions.at(1).z;

			glm::vec3 cameraTar;
			cameraTar.x = (1 - i) * targets.at(0).x + i * targets.at(1).x;
			cameraTar.y = (1 - i) * targets.at(0).y + i * targets.at(1).y;
			cameraTar.z = (1 - i) * targets.at(0).z + i * targets.at(1).z;

			fogDensityValue = (1 - i) * fogs.at(0) + i * fogs.at(1);

			myCamera.cameraPosition = cameraPos;
			myCamera.cameraTarget = cameraTar;

			view = myCamera.getViewMatrix();
			myBasicShader.useShaderProgram();
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

			renderScene();
			glCheckError();
			glfwSwapBuffers(myWindow.getWindow());
		}

		positions.erase(positions.begin());
		targets.erase(targets.begin());
		fogs.erase(fogs.begin());
	}
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState(); 
	initModels(); 
	initShaders(); 
	initUniforms();  
    setWindowCallbacks();  

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement(); 
	    renderScene(); 
		glfwPollEvents(); 
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
