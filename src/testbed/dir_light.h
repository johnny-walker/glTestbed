#ifndef DIR_LIGHT_H
#define DIR_LIGHT_H
#include "light.h"

class DirLight : public Light
{
public:
	DirLight(int id, int width, int height);
	~DirLight();

	void init(Shader* pShader, Camera* pCamera);
	void render();
};
#endif //DIR_LIGHT_H

