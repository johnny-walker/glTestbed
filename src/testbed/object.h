#ifndef BASEOBJ_H
#define BASEOBJ_H
#include "opengl.h"

class BaseObject
{
protected:
	Shader* pCurShader = nullptr;
	Camera* pCurCamera = nullptr;

	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	int scrWidth  = 0;
	int scrHeight = 0;
	
	float scale = 1.f;
	float angle = 0.f;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f ,0.0f);
	bool dirty = true;	// flag if angle or pos changes

public:
	BaseObject(int width, int height) 
	{
		scrWidth  = width;
		scrHeight = height;
	}
	~BaseObject() {}

public:
	virtual void init(Shader* pShader, Camera* pCamera);
	virtual void render();
	virtual void setShader(Shader* pShader);

public:
	void setScale(float value);									//init scale
	void setAngle(float rotate);								//init angle
	void setPos(float posX, float posY, float posZ);			//init pos
	void updateAngle(float delta);								//delta
	void updatePos(float deltaX, float deltaY, float deltaZ);	//delta

	float getScale();
	float getAngle();
	glm::vec3 getPos();

	void processInput(GLFWwindow* glWindow, float delta);

protected:
	unsigned int loadTexture(char const* path);
};
#endif //BASEOBJ_H
