#ifndef WORLD_H
#define WORLD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

class World
{
	GLFWwindow* glWindow = NULL;

	Shader* myShader = NULL;
	Model* myModel = NULL;
	Camera* myCamera = NULL;

	// settings
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 600;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	bool autoRotate = false;
	float lastAngel = glm::radians(210.f);

	// scene 
	unsigned int planeVAO = 0;
	unsigned int woodTexture = 0;

public:
	World(GLFWwindow* window=NULL, int width=800, int height=600) {
		glWindow = window;
		SCR_WIDTH = width;
		SCR_HEIGHT = height;
	}

	bool init();
	void render();
	void terminate();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int heit);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

private:
	unsigned int loadTexture(char const* path);
	void processInput();
	void setupFloor();
	void renderFloor();
	void renderModel();
};
#endif //WORLD_H

