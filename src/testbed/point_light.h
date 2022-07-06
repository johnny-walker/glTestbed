#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H
#include "light.h"

class PointLight : public Light
{
private:
	// drawing 
	unsigned int sphereVAO = 0;
	int indexCount = 0;
	float drawScale = 0.15f;

public:
	PointLight(int id, int width, int height, bool cubemap = true);
	~PointLight();

	void init(Shader* pShader, Camera* pCamera);
	void render();
	void initCubemapTexture();

	void drawPointSphere(bool flag);
	std::vector<glm::mat4> createCubemapMatrix(float nearPlane, float farPlane);
	std::vector<glm::mat4> getCubemapMatrix();
	unsigned int getCubemap();
	unsigned int getCubemapFBO();

private:
	bool useCubemap = true;
	unsigned int depthCubemapFBO = 0;
	unsigned int depthCubemap = 0;
	std::vector<glm::mat4> shadowTransforms;

private:
	bool drawSphere = true;
	void initSphere();
};
#endif //POINT_LIGHT_H

