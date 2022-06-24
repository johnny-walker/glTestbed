#include "world.h"
World* thisWorld = nullptr;
float mouseLASTX = 0.f;
float mouseLASTY = 0.f;
bool  mousePRESS = true;

bool World::init() 
{
    // init global variables for callback
    thisWorld = this;
 
    // init openGL
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(glWindow);
    glfwSetFramebufferSizeCallback(glWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(glWindow, mouse_callback);
    glfwSetScrollCallback(glWindow, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    // init shaders
    pShader = new Shader("world.vs", "world.fs");
    pCamera = new Camera(glm::vec3(0.f, 1.0f, 10.f));

    // create scene objects
    pFloor = new Floor(scrWidth, scrHeight);
    pFloor->init(pShader, pCamera);

    pCow = new BaseModel(scrWidth, scrHeight, "../../resources/objects/spot/spot.obj");
    pCow->init(pShader, pCamera);
    pCow->setAngle(glm::radians(210.f));
    pCow->setPos(0.f, 0.25f, 0.f);

    pRobot = new BaseModel(scrWidth, scrHeight, "../../resources/objects/cyborg/cyborg.obj");
    pRobot->init(pShader, pCamera);
    pRobot->setAngle(glm::radians(-45.f));
    pRobot->setPos(3.f, 0.f, -1.f);
    
    pCube = new Cube(scrWidth, scrHeight);
    pCube->init(pShader, pCamera);
    pCube->setAngle(glm::radians(60.f));
    pCube->setPos(-5.f, 0.f, -3.0);
    pCube->setScale(0.5f);

    // init light attributes
    pPtLight = new PointLight(scrWidth, scrHeight);
    pPtLight->init(pShader, pCamera);
    pPtLight->setPos(-1.f, 1.5f, 1.5f);
    pPtLight->setColor(glm::vec3(1.f, 1.f, 1.f));      // white
    pPtLight->setStrength(1.f);      

    pDirLight = new DirLight(scrWidth, scrHeight);
    pDirLight->init(pShader, pCamera);
    pDirLight->setDirection(glm::vec3(1.f, 2.f, 1.f));
    pDirLight->setColor(glm::vec3(1.f, 1.f, 1.f));      // white
    pDirLight->setStrength(0.3f);

    return true;
}

void World::render() 
{
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(glWindow)) {
        pShader->use();
        pShader->setInt("RenderMode", 0);
        pShader->setInt("LightingModel", lightModel);
        pShader->setVec3("ViewPos", pCamera->Position);

        // per-frame time dalta
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(deltaTime);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // must render lights first (in front of objects)
        pPtLight->render();
        pDirLight->render();
        pFloor->render();
        pCube->render();
        pCow->render();
        pRobot->render();

        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
}

void World::terminate() 
{
    glfwTerminate();
}

void World::processInput(float deltaTime)
{
    if (glfwGetKey(glWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glWindow, true);

    // camera pos
    if (glfwGetKey(glWindow, GLFW_KEY_W) == GLFW_PRESS)
        pCamera->ProcessKeyboard(FORWARD, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_S) == GLFW_PRESS)
        pCamera->ProcessKeyboard(BACKWARD, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_A) == GLFW_PRESS)
        pCamera->ProcessKeyboard(LEFT, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_D) == GLFW_PRESS)
        pCamera->ProcessKeyboard(RIGHT, deltaTime);

    // hotkey controls for models/lights
    switch (target) {
    case CTRL_TARGET::COW:
        pCow->processInput(glWindow, deltaTime);
        break;
    case CTRL_TARGET::ROBOT:
        pRobot->processInput(glWindow, deltaTime);
        break;
    case CTRL_TARGET::CUBE:
        pCube->processInput(glWindow, deltaTime);
        break;
    case CTRL_TARGET::POINT_LIGHT:
        pPtLight->processInput(glWindow, deltaTime);
        break;
    case CTRL_TARGET::DIRECTION_LIGHT:
        pDirLight->processInput(glWindow, deltaTime);
        break;
    default:
        break;
    }

    // adjust color
    if (pCtrlLight) {
        pCtrlLight->processLight(glWindow);
    }

    // switch target object for hotkey controls
    if (glfwGetKey(glWindow, GLFW_KEY_F1) == GLFW_PRESS) {
        target = CTRL_TARGET::COW;
        pCtrlLight = nullptr;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F2) == GLFW_PRESS) {
        target = CTRL_TARGET::ROBOT;
        pCtrlLight = nullptr;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F3) == GLFW_PRESS) {
        target = CTRL_TARGET::CUBE;
        pCtrlLight = nullptr;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F5) == GLFW_PRESS) {
        target = CTRL_TARGET::POINT_LIGHT;
        pCtrlLight = pPtLight;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F6) == GLFW_PRESS) {
        target = CTRL_TARGET::DIRECTION_LIGHT;
        pCtrlLight = pDirLight;
    }

    // switch target object for hotkey controls
    if (glfwGetKey(glWindow, GLFW_KEY_F9) == GLFW_PRESS) {
        lightModel = 0;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F10) == GLFW_PRESS) {
        lightModel = 1;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F11) == GLFW_PRESS) {
        lightModel = 2;
    }
}

void World::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void World::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        mousePRESS = true;
        return;
    }
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (mousePRESS) {
            mouseLASTX = xpos;
            mouseLASTY = ypos;
            mousePRESS = false;
        }
    }

    float xoffset = xpos - mouseLASTX;
    float yoffset = mouseLASTY - ypos; // reversed since y-coordinates go from bottom to top

    mouseLASTX = xpos;
    mouseLASTY = ypos;

    if (thisWorld)
        thisWorld->pCamera->ProcessMouseMovement(xoffset, yoffset);
}

void World::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (thisWorld)
        thisWorld->pCamera->ProcessMouseScroll(static_cast<float>(yoffset));
}