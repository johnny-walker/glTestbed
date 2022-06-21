#ifndef BASEOBJ_H
#define BASEOBJ_H
#include "opengl.h"

class BaseObject
{
protected:
	Shader* pCurShader = nullptr;
	Camera* pCurCamera = nullptr;

	int scr_width  = 0;
	int scr_height = 0;
	
	bool dirty = true;	// flag if angle or pos changes
	float angle = 0;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f ,0.0f);

public:
	BaseObject(int width, int height) 
	{
		scr_width  = width;
		scr_height = height;
	}
	~BaseObject() {}

public:
	void setAngle(float rotate);								//init angle
	void setPos(float posX, float posY, float posZ);			//init pos
	void updateAngle(float delta);								//delta
	void updatePos(float deltaX, float deltaY, float deltaZ);	//delta

	virtual void init(Shader* pShader, Camera* pCamera) = 0;
	virtual void render() = 0;

protected:
	unsigned int loadTexture(char const* path);
};
#endif //BASEOBJ_H
