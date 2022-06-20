#include "world.h"
World* thisWorld = NULL;
float gLastX = 400.0f;
float gLastY = 300.0f;
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

    myShader = new Shader("model.vs", "model.fs");
    myCamera = new Camera(glm::vec3(0.0f, 0.0f, 8.0f));

    spot = new Model("../../resources/objects/spot/spot.obj");
    floor = new Floor();
    floor->init(myShader);

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
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        myShader->use();

        // create view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(myCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = myCamera->GetViewMatrix();
        myShader->setMat4("projection", projection);
        myShader->setMat4("view", view);

        floor->render();
        renderModel();
   
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
}

void World::terminate() 
{
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
}

// render the loaded model
void World::renderModel() 
{
    // update model matrix
    if (target == MODEL) {
        switch (operation) {
        case ROTATE_CC:
            lastAngel += deltaTime;
            break;
        case ROTATE_CLOCK:
            lastAngel -= deltaTime;
            break;
        case UP:
            lastPos += deltaTime;
            break;
        case DOWN:
            lastPos -= deltaTime;
            break;
        default: //IDLE
            // do nothing
            break;
        }
    }
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, lastPos, 0.0f));
    model = glm::rotate(model, (float)(lastAngel), glm::vec3(0.0f, 1.0f, 0.0f));

    myShader->setMat4("model", model);
    // draw
    spot->Draw(*myShader);
}

void World::processInput()
{
    if (glfwGetKey(glWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glWindow, true);

    if (glfwGetKey(glWindow, GLFW_KEY_W) == GLFW_PRESS)
        myCamera->ProcessKeyboard(FORWARD, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_S) == GLFW_PRESS)
        myCamera->ProcessKeyboard(BACKWARD, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_A) == GLFW_PRESS)
        myCamera->ProcessKeyboard(LEFT, deltaTime);
    else if (glfwGetKey(glWindow, GLFW_KEY_D) == GLFW_PRESS)
        myCamera->ProcessKeyboard(RIGHT, deltaTime);

    // hotkey controls for models/lights
    if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
        operation = ROTATE_CC;
    } else if (glfwGetKey(glWindow, GLFW_KEY_T) == GLFW_PRESS) {
        operation = ROTATE_CLOCK;
    } else if (glfwGetKey(glWindow, GLFW_KEY_F) == GLFW_PRESS) {
        operation = UP;
    } else if (glfwGetKey(glWindow, GLFW_KEY_G) == GLFW_PRESS) {
        operation = DOWN;
    } else {
        operation = IDLE;
    }

    // switch target object for hotkey controls
    if (glfwGetKey(glWindow, GLFW_KEY_F1) == GLFW_PRESS) {
        target = MODEL;
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

