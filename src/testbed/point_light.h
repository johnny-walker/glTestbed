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
	PointLight(int id, int width, int height) :
		Light(id, width, height) {}
	~PointLight() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();

	void initCubemapTexture();
	std::vector<glm::mat4> createLightSpaceMatrix(float nearPlane, float farPlane);
	std::vector<glm::mat4> getLightSpaceMatrix();
	unsigned int getCubemap();
	unsigned int getCubemapFBO();

private:
	unsigned int depthCubemapFBO = 0;
	unsigned int depthCubemap = 0;
	std::vector<glm::mat4> shadowTransforms;

private:
	void initSphere();
};
#endif //POINT_LIGHT_H

