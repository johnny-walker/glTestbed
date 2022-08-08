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
	// settings
	unsigned int scrWidth = 800;
	unsigned int scrHeight = 600;

	float lastFrame = 0.0f;

	GLFWwindow* glWindow = nullptr;

	Shader* pShaderWorld = nullptr;
	Shader* pShaderShadow = nullptr;
	Shader* pShaderQuad = nullptr;
	Shader* pShaderCubemap = nullptr;

	Camera* pCamera = nullptr;

	bool pbrRendering = true;

	//lights
	std::vector<PointLight*> ptLights; 
	std::vector<DirLight*> dirLights;

	//scene
	Floor* pFloor = nullptr;
	Cube* pCube = nullptr;
	BaseModel* pFirst = nullptr;
	BaseModel* pRobot = nullptr;
	BaseModel* pBird = nullptr;

	//control targets
	int lightModel = 0;
	BaseObject* pCtrlTarget = nullptr;
	Light* pCtrlLight = nullptr;

public:
	World(GLFWwindow* window = nullptr, int width = 800, int height = 600);
	~World();

	bool init();
	void render();
	void terminate();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int heit);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

private:
	bool initModels();
	bool initPBR();
	bool initLights(int count=1);
	void setShader(Shader* pShaderObj);
	void renderScene(bool drawSphere = true);
	void processInput(float deltaTime = 0.f);

private:
	// direction light shadow map
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;

	bool showDepthMap = false;
	bool debugDepthMap = false;

	void createPtCubemapTexture();
	void generatePtCubemap(float nearPlane, float farPlane);
	void configPtLightShadowMap(float farPlane);
	
	void generateDirShadowMap(float nearPlane, float farPlane);
	void configDirLightShadowMap();

	void renderShadowMap();
	void renderQuad();

private:
	// point light cubemap
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthCubemapFBO[2] = { 0 };
	unsigned int depthCubemap[2] = { 0 };
};
#endif //WORLD_H

