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

	Shader* pShaderPBR			= nullptr;
	Shader* pShaderShadow		= nullptr;
	Shader* pShaderCubemap		= nullptr;
	Shader* pShaderEnvCubemap	= nullptr;
	Shader* pShaderIrrConv		= nullptr;
	Shader* pShaderPrefltr		= nullptr;
	Shader* pShaderBRDF 		= nullptr;
	Shader* pShaderBG			= nullptr;

	Camera* pCamera = nullptr;

	//lights
	std::vector<PointLight*> ptLights; 
	std::vector<DirLight*> dirLights;

	//scene
	Floor* pFloor = nullptr;
	Cube* pCube = nullptr;
	BaseModel* pFirst = nullptr;

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
	void initWorld(bool bFloor = true, bool bCube = true);
	void initDirLights(int count = 1);
	void initPtLights(int count = 1);
	bool initModel();
	bool initPBRModel();
	void setShader(Shader* pShaderObj);
	void renderScene(bool drawSphere = false);
	void processInput(float deltaTime = 0.f);

private:
	unsigned int captureFBO = 0;
	unsigned int captureRBO = 0;
	unsigned int hdrTexture = 0;		// IBL texture
	unsigned int envCubemap = 0;		// IBL cube
	unsigned int irradianceMap = 0;		// IBL cube
	unsigned int prefilterMap = 0;		// IBL cube
	unsigned int brdfLUTTextureMap = 0;	// IBL LUT

	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;

	bool showSkybox = true;
	//bool showDepthMap = false;
	//bool debugDepthMap = false;
	
	void initIBLSpecular(char const* filename);

	void createPtCubemapTexture();
	void generatePtCubemap(float nearPlane, float farPlane);
	void configPtLightShadowMap(float farPlane);
	
	void generateDirShadowMap(float nearPlane, float farPlane);
	void configDirLightShadowMap();

	void renderQuad();
	void renderCube();
	void renderSkybox();

private:
	// point light cubemap
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthCubemapFBO[2] = { 0 };
	unsigned int depthCubemap[2] = { 0 };
};
#endif //WORLD_H

