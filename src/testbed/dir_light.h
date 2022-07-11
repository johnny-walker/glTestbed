#ifndef DIR_LIGHT_H
#define DIR_LIGHT_H
#include "light.h"

class DirLight : public Light
{
private:
	// shadow map
	unsigned int depthMapFBO = 0;
	unsigned int depthMap = 0;
	float projNearPlane = 0.1f;
	float projFarPlane = 10.f;
	bool orthographic = true;
	glm::mat4 lightSpaceMtrx = glm::mat4(1.f);

public:
	DirLight(int id, int width, int height);
	~DirLight();

	void init(Shader* pShader, Camera* pCamera);
	void render();

	//shadow map
	void initShadowMapTexture();
	glm::mat4 createMatrix(float nearPlane, float farPlane, bool ortho);
	glm::mat4 getMatrix();
	float getProjNearPlane();
	float getProjFarPlane();
	unsigned int getShadowMap();
	unsigned int getShadowMapFBO();
	bool isOrthoProjection();
};
#endif //DIR_LIGHT_H

