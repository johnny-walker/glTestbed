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
	unsigned int getShadowMap();
	unsigned int getShadowMapFBO();

private:
	unsigned int depthMapFBO = 0;
	unsigned int depthMap = 0;

};
#endif //DIR_LIGHT_H

