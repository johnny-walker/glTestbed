#ifndef DIR_LIGHT_H
#define DIR_LIGHT_H
#include "light.h"

class DirLight : public Light
{
public:
	DirLight(int id, int width, int height) :
		Light(id, width, height) {}
	~DirLight() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();

	void initShadowMapTexture();
	glm::mat4 createMatrix(float nearPlane, float farPlane);
	glm::mat4 getMatrix();
	unsigned int getShadowMap();
	unsigned int getShadowMapFBO();

private:
	unsigned int depthMapFBO = 0;
	unsigned int depthMap = 0;
	glm::mat4 lightSpaceMtrx = glm::mat4(1.f);

};
#endif //DIR_LIGHT_H

