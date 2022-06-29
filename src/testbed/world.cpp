#include "world.h"
World* thisWorld = nullptr;
float mouseLASTX = 0.f;
float mouseLASTY = 0.f;
bool  mousePRESS = true;

World::World(GLFWwindow* window, int width, int height) 
{
    glWindow = window;
    scrWidth = width;
    scrHeight = height;
}

World::~World() 
{
    ptLights.clear();
    dirLights.clear();
    delete pFloor;
    delete pCube;
    delete pCow;
    delete pRobot;
    if (pBird)
        delete pBird;
}

bool World::init() 
{
    // init global variables for callback
    thisWorld = this;
 
    // init openGL
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(glWindow);
    // setup callback
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
    pShaderShadow = new Shader("shadow_mapping.vs", "shadow_mapping.fs");
    pShaderQuad = new Shader("quad.vs", "quad.fs");
    pShaderCubemap = new Shader("cubemap_depth.vs", "cubemap_depth.fs", "cubemap_depth.gs");

    // specify the texture maps
    pShader->use();
    pShader->setInt("texture_diffuse", 0);
    pShader->setInt("texture_specular", 1);
    pShader->setInt("texture_normal", 2);

    pShaderShadow->use();
    pShaderShadow->setInt("texture_diffuse", 0);
    pShaderShadow->setInt("shadowMap", 1);

    // init camera
    pCamera = new Camera(glm::vec3(0.f, 1.0f, 10.f));

    // create scene objects
    pFloor = new Floor(scrWidth, scrHeight);
    pFloor->init(pShader, pCamera);

    pCow = new BaseModel(scrWidth, scrHeight, "../../resources/objects/spot/spot.obj");
    pCow->init(pShader, pCamera);
    pCow->setAngle(glm::radians(210.f), 1);
    pCow->setPos(0.f, 0.25f, 0.f);
    pCtrlTarget = pCow;

    pRobot = new BaseModel(scrWidth, scrHeight, "../../resources/objects/cyborg/cyborg.obj");
    pRobot->init(pShader, pCamera);
    pRobot->setAngle(glm::radians(-45.f));
    pRobot->setPos(3.f, -0.5f, -1.f);
    
    pCube = new Cube(scrWidth, scrHeight);
    pCube->init(pShader, pCamera);
    pCube->setAngle(glm::radians(60.f), 1);
    pCube->setPos(-3.5f, 0.f, -3.5f);
    pCube->setScale(0.5f);

    bool showBird = false; //loading bird takes time, false to save time
    if (showBird) { 
        pBird = new BaseModel(scrWidth, scrHeight, "../../resources/objects/bird/12213_Bird_v1_l3.obj");
        pBird->init(pShader, pCamera);
        pBird->setAngle(glm::radians(-90.f), 0);
        pBird->setAngle(glm::radians(75.f), 2);
        pBird->setPos(-30.f, -6.f, 35.f);
        pBird->setScale(0.08f);
    }

    // init 2 point lights and 2 direction lights
    pShader->use();
    pShader->setInt("ptLightCount", 2);
    pShader->setInt("dirLightCount", 2);

    PointLight* pPtLight = new PointLight(0, scrWidth, scrHeight);
    pPtLight->init(pShader, pCamera);
    pPtLight->setPos(-1.f, 1.5f, 1.5f);
    pPtLight->setPrimaryColor(2);   //orange      
    pPtLight->setStrength(1.f);
    ptLights.push_back(pPtLight);

    pPtLight = new PointLight(1, scrWidth, scrHeight);
    pPtLight->init(pShader, pCamera);
    pPtLight->setPos(-1.f, 1.5f, -3.5f);
    pPtLight->setPrimaryColor(4);   //green      
    pPtLight->setStrength(1.f);
    ptLights.push_back(pPtLight);

    DirLight* pDirLight = new DirLight(0, scrWidth, scrHeight);
    pDirLight->init(pShader, pCamera);
    pDirLight->setPos(2.f, 2.f, 3.f);
    pDirLight->setPrimaryColor(0);  //white
    pDirLight->setStrength(1.f);
    dirLights.push_back(pDirLight);

    pDirLight = new DirLight(1, scrWidth, scrHeight);
    pDirLight->init(pShader, pCamera);
    pDirLight->setPos(2.f, 2.f, -3.f);
    pDirLight->setPrimaryColor(0);  //white
    pDirLight->setStrength(1.f);
    dirLights.push_back(pDirLight);

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
        // update user inputs
        setShader(pShader);
        pShader->setInt("renderMode", 0);
        pShader->setInt("lightingModel", lightModel);
        pShader->setVec3("viewPos", pCamera->Position);

        // render scene
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1, genearte shadow mapping
        // 1.1, generate direction light shadow map
        float nearPlane = 0.1f, farPlane = 10.5f;
        unsigned int depthMapFBO = 0;
        unsigned int depthMap = 0;
        glm::mat4 lightMtrx = glm::mat4(0.f);
            
        setShader(pShaderShadow);
        for (int i = 0; i < dirLights.size(); i++) {
            lightMtrx = dirLights[i]->createLightSpaceMatrix(nearPlane, farPlane);
            pShaderShadow->setMat4("lightSpaceMatrix", lightMtrx);
            depthMapFBO = dirLights[i]->getShadowMapFBO();

            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);
                renderScene();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        //1.2, generate point light cubemap shadow
        unsigned int cubemapFBO = 0;
        unsigned int cubemap = 0;
        float ptNearPlane = 1.f, ptFarPlane = 25.f;
        std::vector<glm::mat4> shadowMatrices;

        setShader(pShaderCubemap);
        for (int i = 2; i < ptLights.size(); i++) {
            shadowMatrices = ptLights[i]->createLightSpaceMatrix(ptNearPlane, ptFarPlane);
            pShaderCubemap->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowMatrices[i]);
            pShaderCubemap->setFloat("farPlane", ptFarPlane);
            pShaderCubemap->setVec3("lightPos", ptLights[i]->getPos());
            
            cubemapFBO = ptLights[i]->getCubemapFBO();
            glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);
                renderScene();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // 2 
        // 2.1, show direction light shadow map
        // 2.2, render scene
        if (showDepthMap) {
            // 2.1, show direction light shadow map
            pShaderQuad->use();
            pShaderQuad->setFloat("nearPlane", nearPlane);
            pShaderQuad->setFloat("farPlane", farPlane);
            for (int i = 0; i < dirLights.size(); i++) {
                if (pCtrlLight == dirLights[i]) {
                    depthMap = dirLights[i]->getShadowMap();
                    break;
                }
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            renderQuad();
        } else {
            // 2.2, render scene
            setShader(pShader);
            for (int i = 0; i < dirLights.size(); i++) {
                lightMtrx = dirLights[i]->getLightSpaceMatrix();
                pShader->setMat4("lightSpaceMatrix" + std::to_string(i), lightMtrx);
                pShader->setInt("shadowMap" + std::to_string(i), 3 + i);
                depthMap = dirLights[i]->getShadowMap();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glActiveTexture(GL_TEXTURE3 + i);
                glBindTexture(GL_TEXTURE_2D, depthMap);
            }
            pShader->setFloat("farPlane", ptFarPlane);
            for (int i = 0; i < ptLights.size(); i++) {
                pShader->setInt("cubeMap" + std::to_string(i), 5 + i);
                cubemap = ptLights[i]->getCubemap();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glActiveTexture(GL_TEXTURE5 + i);
                glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
            }
            renderScene();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
}

void World::setShader(Shader* pShader)
{
    pShader->use();
    for (int i=0; i< ptLights.size(); i++)
        ptLights[i]->setShader(pShader);
    for (int i = 0; i < dirLights.size(); i++)
        dirLights[i]->setShader(pShader);

    pFloor->setShader(pShader);
    pCube->setShader(pShader);
    pCow->setShader(pShader);
    pRobot->setShader(pShader);
    if (pBird)
        pBird->setShader(pShader);
}

void World::renderScene()
{
    // render point lights
    if (lightModel == 0 || lightModel == 1) {
        for (int i = 0; i < ptLights.size(); i++)
            ptLights[i]->render();
    }
    // render direction lights
    if (lightModel == 0 || lightModel == 2) {
        for (int i = 0; i < dirLights.size(); i++)
            dirLights[i]->render();
    }
    pFloor->render();
    pCube->render();
    pCow->render();
    pRobot->render();
    if (pBird)
        pBird->render();
}

void World::terminate() 
{
    glfwTerminate();
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void World::renderQuad()
{
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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
    if (pCtrlTarget) {
        pCtrlTarget->processInput(glWindow, deltaTime);
    }
    // adjust color
    if (pCtrlLight) {
        pCtrlLight->processLight(glWindow);
    }

    // switch target object for hotkey controls
    if (glfwGetKey(glWindow, GLFW_KEY_F1) == GLFW_PRESS) {
        pCtrlTarget = pCow;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F2) == GLFW_PRESS) {
        pCtrlTarget = pRobot;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F3) == GLFW_PRESS) {
        pCtrlTarget = pCube;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F4) == GLFW_PRESS) {
        if (pBird)
            pCtrlTarget = pBird;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F5) == GLFW_PRESS) {
        pCtrlTarget = pCtrlLight = ptLights[0];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F6) == GLFW_PRESS) {
        pCtrlTarget = pCtrlLight = ptLights[1];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F7) == GLFW_PRESS) {
        pCtrlTarget = pCtrlLight = dirLights[0];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F8) == GLFW_PRESS) {
        pCtrlTarget = pCtrlLight = dirLights[1];
    }

    // switch lighting algorithms
    if (glfwGetKey(glWindow, GLFW_KEY_F9) == GLFW_PRESS) {
        // turn on all lights
        lightModel = 0;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F10) == GLFW_PRESS) {
        // turn on point lights only
        lightModel = 1;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F11) == GLFW_PRESS) {
        // turn on direction lights only
        lightModel = 2;
    }

    if (glfwGetKey(glWindow, GLFW_KEY_Z) == GLFW_PRESS) {
        showDepthMap = true;
    }
    else {
        showDepthMap = false;
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