#ifndef WORLD_H
#define WORLD_H
#include "opengl.h"
#include "floor.h"

class World
{
private:
	enum MODEL_OP {
		IDLE = 0,
		ROTATE_CC = 1,	// counter clockwise
		ROTATE_CLOCK = 2,	// clockwise
		UP = 3,
		DOWN = 4
	};

	enum CONTROL_TARGET {
		MODEL = 0,	// spot (cow)
		LIGHT1 = 1,	// parallel light
		LIGHT2 = 2,	// point light
	};

public:
	GLFWwindow* glWindow = NULL;

	Shader* myShader = NULL;
	Camera* myCamera = NULL;

	//scene
	Floor* floor = NULL;
	Model* spot = NULL;

	// settings
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 600;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float lastAngel = glm::radians(210.f);
	float lastPos = 0.25f;

	MODEL_OP operation = IDLE;
	CONTROL_TARGET target = MODEL;

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
	void renderModel();
};
#endif //WORLD_H

