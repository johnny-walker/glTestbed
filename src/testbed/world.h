#ifndef WORLD_H
#define WORLD_H
#include "opengl.h"
#include "floor.h"
#include "cow.h"
#include "point_light.h"
#include "parallel_light.h"

class World
{
private:
	enum class CTRL_TARGET {
		COW,				// model spot
		POINT_LIGHT,		// point light
		PARALLEL_LIGHT,		// parallel light
	};

	// settings
	int scrWidth = 800;
	int scrHeight = 600;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	GLFWwindow* glWindow = nullptr;

	Shader* pShader = nullptr;
	Camera* pCamera = nullptr;

	//scene
	Floor* pFloor = nullptr;
	Cow* pCow = nullptr;
	PointLight* pPtLight = nullptr;
	ParallelLight* pParalLight = nullptr;

	//control targets
	CTRL_TARGET target = CTRL_TARGET::COW;
	Light* pCtrlLight = nullptr;

public:
	World(GLFWwindow* window=nullptr, int width=800, int height=600) {
		glWindow = window;
		scrWidth = width;
		scrHeight = height;
	}
	~World() {
	}

public:
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

