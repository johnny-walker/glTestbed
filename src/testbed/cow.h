#ifndef Cow_H
#define Cow_H
#include "object.h"

class Cow: public BaseObject
{
public: 
	Cow(int width, int height, string const& path, bool gamma = false) :
		BaseObject(width, height)
	{
		//flip loaded texture's on the y-axis (st coordinate)
		//stbi_set_flip_vertically_on_load(true);
		spot = new Model(path, gamma);
	}
	~Cow() 
	{
		delete spot;
	}

public:
	void init(Shader* pShader, Camera* pCamera);
	void render();
	void processInput(GLFWwindow* glWindow, float delta);

private:
	Model* spot = nullptr;	// this cow's name is spot
};
#endif //Cow

