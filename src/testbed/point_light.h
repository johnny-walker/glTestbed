#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H
#include "light.h"

class PointLight : public Light
{
private:
	// drawing 
	unsigned int sphereVAO = 0;
	int indexCount = 0;
	float drawScale = 0.05f;

public:
	PointLight(int id, int width, int height);
	~PointLight();

	void init(Shader* pShader, Camera* pCamera);
	void render();

	void drawPointSphere(bool flag);
	std::vector<glm::mat4> createCubemapMatrix(float aspect, float nearPlane, float farPlane);
	std::vector<glm::mat4> getCubemapMatrix();

private:
	std::vector<glm::mat4> shadowTransforms;

private:
	bool drawSphere = true;
	void initSphere();
};
#endif //POINT_LIGHT_H

