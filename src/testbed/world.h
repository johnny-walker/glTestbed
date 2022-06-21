#ifndef WORLD_H
#define WORLD_H
#include "opengl.h"
#include "floor.h"
#include "cow.h"

class World
{
private:
	enum CONTROL_TARGET {
		COW = 0,	// model spot
		LIGHT1 = 1,	// parallel light
		LIGHT2 = 2,	// point light
	};

public:
	GLFWwindow* glWindow = NULL;

	Shader* pShader = NULL;
	Camera* pCamera = NULL;

	//scene
	Floor* floor = NULL;
	Cow* cow = NULL;

	// settings
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 600;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	CONTROL_TARGET target = COW;

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
	void processInput();
};
#endif //WORLD_H

