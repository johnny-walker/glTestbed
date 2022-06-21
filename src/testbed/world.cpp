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
 
    // init colors
    colors.push_back(glm::vec3(1.f, 1.f, 1.f));         //white (255, 255, 255)
    colors.push_back(glm::vec3(1.f, 0.f, 0.f));         //red (255, 0, 0)
    colors.push_back(glm::vec3(1.f, 165.f/255.f, 0.f)); //orange (255, 165, 0)
    colors.push_back(glm::vec3(1.f, 1.f, 0.f));         //yellow (255,255,0)
    colors.push_back(glm::vec3(0.f, 0.5f, 0.f));        //green (0,128,0)
    colors.push_back(glm::vec3(0.f, 1.f, 1.f));         //cyan (0,255,255)
    colors.push_back(glm::vec3(0.f, 0.f, 1.f));         //blue (0,0,255)
    colors.push_back(glm::vec3(0.5f, 0.f, 0.5f));       //purple (128,0,128)
    colors.push_back(glm::vec3(1.f, 0.f, 1.f));         //magenta (255, 0, 255)

    // init openGL
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(glWindow);
    glfwSetFramebufferSizeCallback(glWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(glWindow, mouse_callback);
    glfwSetScrollCallback(glWindow, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    // init shaders
    pShader = new Shader("world.vs", "world.fs");
    pCamera = new Camera(glm::vec3(0.0f, 0.0f, 8.0f));

    // create scene objects
    floor = new Floor(scrWidth, scrHeight);
    floor->init(pShader, pCamera);

    cow = new Cow(scrWidth, scrHeight, "../../resources/objects/spot/spot.obj");
    cow->setAngle(glm::radians(150.f));
    cow->setPos(0.f, 0.25f, 0.f);
    cow->init(pShader, pCamera);

    // init light attributes
    ptLight = new PointLight(scrWidth, scrHeight);
    ptLight->setPos(1.f, 1.f, 2.f);
    ptLight->setColor(glm::vec3(1.f, 1.f, 1.f));
    ptLight->init(pShader, pCamera);

    return true;
}

void World::render() 
{
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(glWindow)) {
        // per-frame time dalta
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ptLight->render();
        floor->render();
        cow->render();
   
        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
}

void World::terminate() 
{
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
}

void World::processInput()
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
        if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
            cow->updateAngle(deltaTime);        //counter clockwise
        } else if (glfwGetKey(glWindow, GLFW_KEY_T) == GLFW_PRESS) {
            cow->updateAngle(-deltaTime);       //clockwise
        } else if (glfwGetKey(glWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cow->updatePos(deltaTime, 0, 0);    //x:left
        } else if (glfwGetKey(glWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cow->updatePos(-deltaTime, 0, 0);   //x:right
        } else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            cow->updatePos(0, deltaTime, 0);    //y:up
        } else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            cow->updatePos(0, -deltaTime, 0);   //y:down
        } else if (glfwGetKey(glWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cow->updatePos(0, 0, deltaTime);    //z:ahead
        } else if (glfwGetKey(glWindow, GLFW_KEY_UP) == GLFW_PRESS) {
            cow->updatePos(0, 0, -deltaTime);   //z:back
        }
        break;
    case CTRL_TARGET::POINT_LIGHT:
        if (glfwGetKey(glWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
            ptLight->updatePos(deltaTime, 0, 0);    //x:left
        }
        else if (glfwGetKey(glWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            ptLight->updatePos(-deltaTime, 0, 0);   //x:right
        }
        else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            ptLight->updatePos(0, deltaTime, 0);    //y:up
        }
        else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            ptLight->updatePos(0, -deltaTime, 0);   //y:down
        }
        else if (glfwGetKey(glWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
            ptLight->updatePos(0, 0, deltaTime);    //z:ahead
        }
        else if (glfwGetKey(glWindow, GLFW_KEY_UP) == GLFW_PRESS) {
            ptLight->updatePos(0, 0, -deltaTime);   //z:back
        }
        break;
    default:
        break;
    }

    if (glfwGetKey(glWindow, GLFW_KEY_0) == GLFW_PRESS) {
        ptLight->setColor(colors[0]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_1) == GLFW_PRESS) {
        ptLight->setColor(colors[1]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_2) == GLFW_PRESS) {
        ptLight->setColor(colors[2]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_3) == GLFW_PRESS) {
        ptLight->setColor(colors[3]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_4) == GLFW_PRESS) {
        ptLight->setColor(colors[4]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_5) == GLFW_PRESS) {
        ptLight->setColor(colors[5]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_6) == GLFW_PRESS) {
        ptLight->setColor(colors[6]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_7) == GLFW_PRESS) {
        ptLight->setColor(colors[7]);
    } else if (glfwGetKey(glWindow, GLFW_KEY_8) == GLFW_PRESS) {
        ptLight->setColor(colors[8]);
    }
    
    // switch target object for hotkey controls
    if (glfwGetKey(glWindow, GLFW_KEY_F1) == GLFW_PRESS) {
        target = CTRL_TARGET::COW;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F2) == GLFW_PRESS) {
        target = CTRL_TARGET::POINT_LIGHT;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F3) == GLFW_PRESS) {
        target = CTRL_TARGET::PARALLEL_LIGHT;
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

