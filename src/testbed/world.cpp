#include "world.h"
World* thisWorld = NULL;
float gLastX = 0.f;
float gLastY = 0.f;
bool  gFirstMouse = true;

bool World::init() 
{
    // tell GLFW to capture our mouse
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);
    glfwSetFramebufferSizeCallback(glWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(glWindow, mouse_callback);
    glfwSetScrollCallback(glWindow, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    myShader = new Shader("world.vs", "world.fs");
    myCamera = new Camera(glm::vec3(0.0f, 0.0f, 8.0f));

    floor = new Floor(SCR_WIDTH, SCR_HEIGHT);
    floor->init(myShader, myCamera);

    cow = new Cow(SCR_WIDTH, SCR_HEIGHT, "../../resources/objects/spot/spot.obj");
    cow->init(myShader, myCamera);
    cow->setAngle(glm::radians(150.f));
    cow->setPos(0.f, 0.25f, 0.f);

    // init global variables for callback
    thisWorld = this;
    gLastX = SCR_WIDTH  / 2.0f;
    gLastY = SCR_HEIGHT / 2.0f;
    gFirstMouse = true;

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
        myCamera->ProcessKeyboard(FORWARD, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_S) == GLFW_PRESS)
        myCamera->ProcessKeyboard(BACKWARD, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_A) == GLFW_PRESS)
        myCamera->ProcessKeyboard(LEFT, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_D) == GLFW_PRESS)
        myCamera->ProcessKeyboard(RIGHT, deltaTime);

    // hotkey controls for models/lights
    switch (target) {
    case COW:
        if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
            cow->updateAngle(deltaTime);//counter clockwise
        } else if (glfwGetKey(glWindow, GLFW_KEY_T) == GLFW_PRESS) {
            cow->updateAngle(-deltaTime);//clockwise
        } else if (glfwGetKey(glWindow, GLFW_KEY_F) == GLFW_PRESS) {
            cow->updatePos(deltaTime, 0, 0);//x:left
        } else if (glfwGetKey(glWindow, GLFW_KEY_G) == GLFW_PRESS) {
            cow->updatePos(-deltaTime, 0, 0);//x:right
        } else if (glfwGetKey(glWindow, GLFW_KEY_H) == GLFW_PRESS) {
            cow->updatePos(0, deltaTime, 0);//y:up
        } else if (glfwGetKey(glWindow, GLFW_KEY_I) == GLFW_PRESS) {
            cow->updatePos(0, -deltaTime, 0);//y:down
        } else if (glfwGetKey(glWindow, GLFW_KEY_J) == GLFW_PRESS) {
            cow->updatePos(0, 0, deltaTime);//z:ahead
        } else if (glfwGetKey(glWindow, GLFW_KEY_K) == GLFW_PRESS) {
            cow->updatePos(0, 0, -deltaTime);//z:back
        }
        break;
    default:
        break;
    }

    // switch target object for hotkey controls
    if (glfwGetKey(glWindow, GLFW_KEY_F1) == GLFW_PRESS) {
        target = COW;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F2) == GLFW_PRESS) {
        target = LIGHT1;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F3) == GLFW_PRESS) {
        target = LIGHT2;
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

    if (gFirstMouse) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    if (thisWorld)
        thisWorld->myCamera->ProcessMouseMovement(xoffset, yoffset);
}

void World::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (thisWorld)
        thisWorld->myCamera->ProcessMouseScroll(static_cast<float>(yoffset));
}

