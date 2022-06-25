#ifndef WORLD_H
#define WORLD_H
#include "opengl.h"
#include "floor.h"
#include "cube.h"
#include "model_base.h"
#include "point_light.h"
#include "dir_light.h"

class World
{
private:
	enum class CTRL_TARGET {
		COW,				// model spot
		ROBOT,				// model robot
		CUBE,				// cube
		POINT_LIGHT,		// point light
		DIRECTION_LIGHT,	// direction light
	};

	// settings
	int scrWidth = 800;
	int scrHeight = 600;

	float lastFrame = 0.0f;

	GLFWwindow* glWindow = nullptr;
	Shader* pShader = nullptr;
	Camera* pCamera = nullptr;

	//lights
	PointLight* pPtLight = nullptr;
	DirLight* pDirLight = nullptr;

	//scene
	Floor* pFloor = nullptr;
	Cube* pCube = nullptr;
	BaseModel* pCow = nullptr;
	BaseModel* pRobot = nullptr;

	//control targets
	CTRL_TARGET target = CTRL_TARGET::COW;
	Light* pCtrlLight = nullptr;
	int lightModel = 0;	

public:
	World(GLFWwindow* window=nullptr, int width=800, int height=600) {
		glWindow = window;
		scrWidth = width;
		scrHeight = height;
	}
	~World() {
		delete pPtLight;
		delete pDirLight;
		delete pFloor;
		delete pCube;
		delete pCow;
		delete pRobot;
	}

public:
	bool init();
	void render();
	void terminate();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int heit);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

private:
	void setShader(Shader* pShader);
	void renderScene();
	void processInput(float deltaTime = 0.f);

private:
	// shadow map
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	unsigned int depthMapFBO = 0;
	unsigned int depthMap = 0;

	Shader* pShaderShadow = nullptr;
	Shader* pShaderQuad = nullptr;

	void initShadowMapTexture();
	void configShadowMap(glm::vec3 lightPos);
	void renderQuad();
};
#endif //WORLD_H

