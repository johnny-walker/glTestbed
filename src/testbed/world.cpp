#include "world.h"
World* thisWorld = nullptr;
float LASTX = 0.f;
float LASTY = 0.f;
bool  INIT_MOUSE = true;

bool World::init() 
{
    // init global variables for callback
    thisWorld = this;
    LASTX = scrWidth  / 2.f;
    LASTY = scrHeight / 2.f;
    INIT_MOUSE = true;
 
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
    pCamera = new Camera(glm::vec3(0.0f, 0.0f, 8.0f));

    // create scene objects
    pFloor = new Floor(scrWidth, scrHeight);
    pFloor->init(pShader, pCamera);

    pCow = new Cow(scrWidth, scrHeight, "../../resources/objects/spot/spot.obj");
    pCow->setAngle(glm::radians(150.f));
    pCow->setPos(0.f, 0.25f, 0.f);
    pCow->init(pShader, pCamera);

    // init light attributes
    pPtLight = new PointLight(scrWidth, scrHeight);
    pPtLight->setPos(-1.f, 1.f, 2.f);
    pPtLight->setColor(glm::vec3(1.f, 1.f, 0.f));       // yellow
    pPtLight->setStrength(0.8f);      
    pPtLight->init(pShader, pCamera);

    pParalLight = new ParallelLight(scrWidth, scrHeight);
    pParalLight->setDirection(glm::vec3(1.f, 2.f, 1.f));     
    pParalLight->setColor(glm::vec3(1.f, 1.f, 1.f));    // white
    pParalLight->setStrength(1.f);
    pParalLight->init(pShader, pCamera);

    return true;
}

void World::render() 
{
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(glWindow)) {
        // per-frame time dalta
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(deltaTime);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // must render lights first (in front of objects)
        pPtLight->render();
        pParalLight->render();
        pFloor->render();
        pCow->render();
   
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
    case CTRL_TARGET::POINT_LIGHT:
        pPtLight->processInput(glWindow, deltaTime);
        break;
    case CTRL_TARGET::PARALLEL_LIGHT:
        pParalLight->processInput(glWindow, deltaTime);
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
        target = CTRL_TARGET::POINT_LIGHT;
        pCtrlLight = pPtLight;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F3) == GLFW_PRESS) {
        target = CTRL_TARGET::PARALLEL_LIGHT;
        pCtrlLight = pParalLight;
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
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (INIT_MOUSE) {
        LASTX = xpos;
        LASTY = ypos;
        INIT_MOUSE = false;
    }

    float xoffset = xpos - LASTX;
    float yoffset = LASTY - ypos; // reversed since y-coordinates go from bottom to top

    LASTX = xpos;
    LASTY = ypos;

    if (thisWorld)
        thisWorld->pCamera->ProcessMouseMovement(xoffset, yoffset);
}

void World::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (thisWorld)
        thisWorld->pCamera->ProcessMouseScroll(static_cast<float>(yoffset));
}

