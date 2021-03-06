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
    if (pFloor) delete pFloor;
    if (pCube)  delete pCube;
    if (pCow)   delete pCow;
    if (pRobot) delete pRobot;
    if (pBird)  delete pBird;
}

bool World::init() 
{
    // init global variables for callback
    thisWorld = this;
 
    // init openGL
    glfwMakeContextCurrent(glWindow);

    // keyboard callback
    glfwSetFramebufferSizeCallback(glWindow, framebuffer_size_callback);

    // mouse callbacks
    //glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetCursorPosCallback(glWindow, mouse_callback);
    //glfwSetScrollCallback(glWindow, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // init shaders
    pShaderWorld = new Shader("world.vs", "world.fs");
    pShaderShadow = new Shader("shadow_mapping.vs", "shadow_mapping.fs");
    pShaderQuad = new Shader("quad.vs", "quad.fs");
    pShaderCubemap = new Shader("cubemap_depth.vs", "cubemap_depth.fs", "cubemap_depth.gs");

    // specify the texture maps
    pShaderWorld->use();
    pShaderWorld->setInt("lightId", (int)-1);

    if (pShaderShadow) {
        pShaderShadow->use();
        pShaderShadow->setInt("texture_diffuse", 0);
        pShaderShadow->setInt("shadowMap", 1);
    }

    // init camera
    pCamera = new Camera(glm::vec3(0.f, 1.0f, 10.f));

    // create scene objects
    pFloor = new Floor(scrWidth, scrHeight);
    pFloor->init(pShaderWorld, pCamera);

    pCow = new BaseModel(scrWidth, scrHeight, "../../resources/objects/spot/spot.obj");
    pCow->init(pShaderWorld, pCamera);
    pCow->setAngle(glm::radians(210.f), 1);
    pCow->setPos(0.f, 0.25f, 0.f);
    pCtrlTarget = pCow;

    pRobot = new BaseModel(scrWidth, scrHeight, "../../resources/objects/cyborg/cyborg.obj");
    pRobot->init(pShaderWorld, pCamera);
    pRobot->setAngle(glm::radians(-45.f));
    pRobot->setPos(3.f, -0.5f, -1.f);

    pCube = new Cube(scrWidth, scrHeight);
    pCube->init(pShaderWorld, pCamera);
    pCube->setAngle(glm::radians(60.f), 1);
    pCube->setPos(-6.5f, 0.f, -5.5f);
    pCube->setScale(0.5f);

    bool showBird = false; //loading bird takes time, false to save time
    if (showBird) { 
        pBird = new BaseModel(scrWidth, scrHeight, "../../resources/objects/bird/12213_Bird_v1_l3.obj");
        pBird->init(pShaderWorld, pCamera);
        pBird->setAngle(glm::radians(-90.f), 0);
        pBird->setAngle(glm::radians(75.f), 2);
        pBird->setPos(-30.f, -6.f, 35.f);
        pBird->setScale(0.08f);
    }

    // init point lights and direction lights
    lightModel = 0;
    pShaderWorld->use();

    PointLight* pPtLight = new PointLight(0, scrWidth, scrHeight);
    pPtLight->init(pShaderWorld, pCamera);
    pPtLight->setPos(-1.f, 1.f, 2.f);
    pPtLight->setPrimaryColor(2);   //orange      
    pPtLight->setStrength(1.f);
    ptLights.push_back(pPtLight);
    
    pPtLight = new PointLight(1, scrWidth, scrHeight);
    pPtLight->init(pShaderWorld, pCamera);
    pPtLight->setPos(-1.5f, 1.5f, -5.5f);
    pPtLight->setPrimaryColor(4);   //green      
    pPtLight->setStrength(0.5f);
    ptLights.push_back(pPtLight);

    pShaderWorld->setInt("ptLights.count", (int) ptLights.size());

    DirLight* pDirLight = new DirLight(0, scrWidth, scrHeight);

    pDirLight->init(pShaderWorld, pCamera);
    pDirLight->setPos(2.f, 2.f, 3.f);
    pDirLight->setPrimaryColor(0);  //white
    pDirLight->setStrength(1.f);
    dirLights.push_back(pDirLight);
    
    pDirLight = new DirLight(1, scrWidth, scrHeight);
    pDirLight->init(pShaderWorld, pCamera);
    pDirLight->setPos(2.f, 2.f, -3.f);
    pDirLight->setPrimaryColor(0);  //white
    pDirLight->setStrength(1.f);
    dirLights.push_back(pDirLight);

    pShaderWorld->setInt("dirLights.count", (int) dirLights.size());

    return true;
}

void World::render() 
{
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    createPtCubemapTexture();

    while (!glfwWindowShouldClose(glWindow)) {
        // per-frame time dalta
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(deltaTime);

        setShader(pShaderWorld);
        pShaderWorld->setInt("lightingModel", lightModel);
        pShaderWorld->setVec3("viewPos", pCamera->Position);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
        float dirNearPlane = 0.1f, dirFarPlane = 10.f;
        float ptNearPlane = 1.f, ptFarPlane = 25.f;

        generateDirShadowMap(dirNearPlane, dirFarPlane);
        generatePtCubemap(ptNearPlane, ptFarPlane);

        if (showDepthMap) {
            renderShadowMap();
        } else {
            // render scene
            setShader(pShaderWorld);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            configDirLightShadowMap();
            configPtLightShadowMap(ptFarPlane);
            renderScene();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
}

void World::createPtCubemapTexture()
{
    for (int i = 0; i < ptLights.size(); i++) {
        glGenFramebuffers(1, &depthCubemapFBO[i]);
        glGenTextures(1, &depthCubemap[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
        for (unsigned int face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT,
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO[i]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void World::generatePtCubemap(float nearPlane, float farPlane)
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    setShader(pShaderCubemap);
    pShaderCubemap->setFloat("farPlane", farPlane);

    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    for (int i = 0; i < ptLights.size(); i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO[i]);
        glClear(GL_DEPTH_BUFFER_BIT);

        std::vector<glm::mat4> shadowMatrices = ptLights[i]->createCubemapMatrix(aspect, nearPlane, farPlane);

        for (int face = 0; face < 6; face++) {
            pShaderCubemap->setMat4("shadowMatrices[" + std::to_string(face) + "]", shadowMatrices[face]);
        }
        pShaderCubemap->setVec3("lightPos", ptLights[i]->getPos());

        renderScene(false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glViewport(0, 0, scrWidth, scrHeight);  // restore to screen resolution
}

void World::configPtLightShadowMap(float farPlane)
{
    // config cubemap
    pShaderWorld->setFloat("ptLights.farPlane", farPlane);
    pShaderWorld->setBool("ptLights.debug", debugDepthMap);
    for (int i = 0; i < ptLights.size(); i++) {
        pShaderWorld->setVec3("ptLights.position[" + std::to_string(i) + "]", ptLights[i]->getPos());
        pShaderWorld->setVec3("ptLights.color[" + std::to_string(i) + "]", ptLights[i]->getColor() * ptLights[i]->getStrength());

        pShaderWorld->setInt("ptLights.cubeMap" + std::to_string(i), 5 + i);
        glActiveTexture(GL_TEXTURE5 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
    }
}

void World::generateDirShadowMap(float nearPlane, float farPlane)
{
    glm::mat4 lightMtrx = glm::mat4(0.f);
    unsigned int depthMapFBO = 0;
    setShader(pShaderShadow);
    for (int i = 0; i < dirLights.size(); i++) {
        //orthographic projection
        lightMtrx = dirLights[i]->createMatrix(nearPlane, farPlane, true);
        pShaderShadow->setMat4("lightSpaceMatrix", lightMtrx);

        depthMapFBO = dirLights[i]->getShadowMapFBO();
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        renderScene(false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void World::configDirLightShadowMap()
{
    pShaderWorld->use();
    pShaderWorld->setBool("dirLights.debug", debugDepthMap);
    for (int i = 0; i < dirLights.size(); i++) {
        glm::mat4 lightMtrx = dirLights[i]->getMatrix();
        pShaderWorld->setMat4("dirLights.matrics[" + std::to_string(i) + "]", lightMtrx);
        pShaderWorld->setVec3("dirLights.direction[" + std::to_string(i) + "]", dirLights[i]->getPos());
        pShaderWorld->setVec3("dirLights.color[" + std::to_string(i) + "]", dirLights[i]->getColor()*dirLights[i]->getStrength());

        unsigned int depthMap = dirLights[i]->getShadowMap();
        pShaderWorld->setInt("dirLights.shadowMap" + std::to_string(i), 3 + i);
        glActiveTexture(GL_TEXTURE3 + i);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }
}

//show direction light shadow map
void World::renderShadowMap()
{
    DirLight* pdirLight = nullptr;
    for (int i = 0; i < dirLights.size(); i++) {
        if (dirLights[i] == pCtrlLight) {
            pdirLight = dirLights[i];
            break;
        }
    }
    if (pdirLight) {
        // ortho {true: linear, false: nonlinear}
        bool ortho = pdirLight->isOrthoProjection();
        unsigned int depthMap = pdirLight->getShadowMap();
        float nearPlane = pdirLight->getProjNearPlane();
        float farPlane = pdirLight->getProjFarPlane();

        pShaderQuad->use();
        pShaderQuad->setBool("convert2Linear", !ortho); 
        pShaderQuad->setFloat("nearPlane", nearPlane);
        pShaderQuad->setFloat("farPlane", farPlane);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderQuad();
    }
}

void World::setShader(Shader* pShaderObj)
{
    pShaderObj->use();
    for (int i=0; i< ptLights.size(); i++)
        ptLights[i]->setShader(pShaderObj);
    for (int i = 0; i < dirLights.size(); i++)
        dirLights[i]->setShader(pShaderObj);
    
    if (pFloor) pFloor->setShader(pShaderObj);
    if (pCube)  pCube->setShader(pShaderObj);
    if (pCow)   pCow->setShader(pShaderObj);
    if (pRobot) pRobot->setShader(pShaderObj);
    if (pBird)  pBird->setShader(pShaderObj);
}

void World::renderScene(bool drawSphere)
{
    // render point lights sphere 
    if (drawSphere) {
        if (lightModel == 0 || lightModel == 1) {
            for (int i = 0; i < ptLights.size(); i++)
                ptLights[i]->render();
        }
    }
    
    if (pFloor) pFloor->render();
    if (pCow)   pCow->render();
    if (pRobot) pRobot->render();
    if (pCube)  pCube->render();
    if (pBird)  pBird->render();
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
        if (ptLights.size() > 0)
            pCtrlTarget = pCtrlLight = ptLights[0];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F6) == GLFW_PRESS) {
        if (ptLights.size() > 1)
            pCtrlTarget = pCtrlLight = ptLights[1];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F7) == GLFW_PRESS) {
        if (dirLights.size() > 0)
            pCtrlTarget = pCtrlLight = dirLights[0];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F8) == GLFW_PRESS) {
        if (dirLights.size() > 1)
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

    showDepthMap  = (glfwGetKey(glWindow, GLFW_KEY_Z) == GLFW_PRESS) ? true : false;
    debugDepthMap = (glfwGetKey(glWindow, GLFW_KEY_X) == GLFW_PRESS) ? true : false;

    if (glfwGetKey(glWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
        //todo
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