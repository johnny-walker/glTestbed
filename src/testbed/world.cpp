#include "world.h"
World* thisWorld = NULL;
float gLastX = 400.0f;
float gLastY = 300.0f;
bool  gFirstMouse = true;

bool World::init() {

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

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    myShader = new Shader("model.vs", "model.fs");
    myModel = new Model("../../resources/objects/spot/spot.obj");
    myCamera = new Camera(glm::vec3(0.0f, 0.0f, 8.0f));

    setupFloor();

    // setup global variables for callback
    thisWorld = this;
    gLastX = SCR_WIDTH / 2.0f;
    gLastY = SCR_HEIGHT / 2.0f;
    gFirstMouse = true;

    return true;
}

void World::render() {

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(glWindow))
    {
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

        renderFloor();
        renderModel();
   
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
}

void World::terminate() {
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
}
void World::setupFloor() {

    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

    // scene
    unsigned int planeVBO;

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    woodTexture = loadTexture("../../resources/textures/brickwall.jpg");
}

void World::renderFloor()
{
    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    
    // update model matrix
    glm::mat4 model = glm::mat4(1.0f);
    myShader->setMat4("model", model);
    
    // draw 
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// render the loaded model
void World::renderModel() {
    // update model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.25f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    if (autoRotate) {
        lastAngel += deltaTime;
    }
    model = glm::rotate(model, (float)(lastAngel), glm::vec3(0.0f, 1.0f, 0.0f));
    myShader->setMat4("model", model);

    // draw
    myModel->Draw(*myShader);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int World::loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

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

    if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
        autoRotate = true;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_RELEASE) {
        autoRotate = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void World::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

void World::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (gFirstMouse)
    {
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void World::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (thisWorld)
        thisWorld->myCamera->ProcessMouseScroll(static_cast<float>(yoffset));
}

