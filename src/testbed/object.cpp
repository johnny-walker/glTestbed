#include "object.h"
BaseObject::BaseObject(int width, int height)
{
    scrWidth = width;
    scrHeight = height;
}
BaseObject::~BaseObject() {}

void BaseObject::init(Shader* pShader, Camera* pCamera)
{
    pCurShader = pShader;
    pCurCamera = pCamera;
}

void BaseObject::render()
{
    pCurShader->use();

    // create mvp 
    projection = glm::perspective(glm::radians(pCurCamera->Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);
    view = pCurCamera->GetViewMatrix();
    model = glm::mat4(1.0f);

    model = glm::scale(model, glm::vec3(scale));
    model = glm::translate(model, pos);
    if (angle.x != 0.f)
        model = glm::rotate(model, angle.x, glm::vec3(1.0f, 0.0f, 0.0f));
    if (angle.y != 0.f)
        model = glm::rotate(model, angle.y, glm::vec3(0.0f, 1.0f, 0.0f));
    if (angle.z != 0.f)
        model = glm::rotate(model, angle.z, glm::vec3(0.0f, 0.0f, 1.0f));

    pCurShader->setMat4("projection", projection);
    pCurShader->setMat4("view", view);
    pCurShader->setMat4("model", model);

    pCurShader->setBool("specularMap", false);
    pCurShader->setBool("normalMap", false);
    pCurShader->setBool("roughnessMap", false);
    pCurShader->setBool("metallicMap", false);
    pCurShader->setBool("ormMap", false);
    pCurShader->setBool("aoMap", false);
    pCurShader->setBool("transMap", false);
    pCurShader->setBool("envMap", false);

    dirty = false;
}

void BaseObject::setShader(Shader* pShader)
{
    pCurShader = pShader;
}

void BaseObject::setScale(float value) {
    scale = value;
    dirty = true;
}

void BaseObject::setAngle(float rotate, int axis) {
    switch (axis) {
    case 0: //rotate by x
        angle.x = rotate;
        break;
    case 1: //rotate by y
        angle.y = rotate;
        break;
    case 2: //rotate by z
        angle.z = rotate;
        break;
    }
    dirty = true;
}

void BaseObject::setPos(float posX, float posY, float posZ) {
    pos.x += posX;
    pos.y += posY;
    pos.z += posZ;
    dirty = true;
}

void BaseObject::updateAngle(float delta, int axis) {
    switch (axis) {
    case 0: //rotate by x
        angle.x += delta;
        break;
    case 1: //rotate by y
        angle.y += delta;
        break;
    case 2: //rotate by z
        angle.z += delta;
        break;
    }
    dirty = true;
}

void BaseObject::updatePos(float deltaX, float deltaY, float deltaZ) {
    pos.x += deltaX;
    pos.y += deltaY;
    pos.z += deltaZ;
    dirty = true;
}

float BaseObject::getScale()
{
    return scale;
}

glm::vec3 BaseObject::getAngle()
{
    return angle;
}

glm::vec3 BaseObject::getPos()
{
    return pos;
}
unsigned int BaseObject::loadTexture(char const* path)
{
    unsigned int textureID=0;
    glGenTextures(1, &textureID);

    int width=0, height=0, nrComponents=3;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


void BaseObject::processInput(GLFWwindow* glWindow, float delta)
{
    if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
        updateAngle(delta, 1);        //r:counter clockwise of y-axis
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_T) == GLFW_PRESS) {
        updateAngle(-delta, 1);       //t:clockwise of y-axis
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F) == GLFW_PRESS) {
        updateAngle(delta, 0);        //f:counter clockwise of x-axis
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_G) == GLFW_PRESS) {
        updateAngle(-delta, 0);       //g:clockwise of x-axis
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
        updatePos(-delta, 0, 0);    //x:left
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        updatePos(delta, 0, 0);   //x:right
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        updatePos(0, delta, 0);    //y:up
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        updatePos(0, -delta, 0);   //y:down
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
        updatePos(0, 0, delta);    //z:ahead
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_UP) == GLFW_PRESS) {
        updatePos(0, 0, -delta);   //z:back
    }
}

