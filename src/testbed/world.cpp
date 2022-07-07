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
    glEnable(GL_CULL_FACE);

    // init shaders
    pShaderWorld = new Shader("world.vs", "world.fs");
    pShaderShadow = new Shader("shadow_mapping.vs", "shadow_mapping.fs");
    pShaderQuad = new Shader("quad.vs", "quad.fs");
    pShaderCubemap = new Shader("cubemap_depth.vs", "cubemap_depth.fs", "cubemap_depth.gs");

    // specify the texture maps
    pShaderWorld->use();
    pShaderWorld->setInt("texture_diffuse", 0);
    pShaderWorld->setInt("texture_specular", 1);
    pShaderWorld->setInt("texture_normal", 2);
    pShaderWorld->setInt("lightId", (int)-1);

    pShaderShadow->use();
    pShaderShadow->setInt("texture_diffuse", 0);
    pShaderShadow->setInt("shadowMap", 1);

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
    pCube->setPos(-6.5f, 0.f, -12.5f);
    //pCube->setPos(0.f, 0.f, 0.f);
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

    // init 2 point lights and 2 direction lights
    lightModel = 0;
    ptCubemap = true;
    pShaderWorld->use();
    pShaderWorld->setBool("ptLights.cubemap", ptCubemap);

    PointLight* pPtLight = new PointLight(0, scrWidth, scrHeight, ptCubemap);
    pPtLight->init(pShaderWorld, pCamera);
    pPtLight->setPos(-1.f, 1.5f, 1.5f);
    pPtLight->setPrimaryColor(2);   //orange      
    pPtLight->setStrength(1.f);
    ptLights.push_back(pPtLight);
    
    pPtLight = new PointLight(1, scrWidth, scrHeight, ptCubemap);
    pPtLight->init(pShaderWorld, pCamera);
    pPtLight->setPos(-1.f, 1.5f, -3.5f);
    pPtLight->setPrimaryColor(4);   //green      
    pPtLight->setStrength(1.f);
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
        float ptNearPlane = 1.f, ptFarPlane = 10.f;
        float ptCubeNearPlane = 1.f, ptCubeFarPlane = 25.f;
        generateDirShadowMap(dirNearPlane, dirFarPlane);
        if (ptCubemap)
            generatePtCubemap(ptCubeNearPlane, ptCubeFarPlane);
        else
            generatePtShadowMap(ptNearPlane, ptFarPlane);

        if (showDepthMap) {
            renderShadowMap();
        } else { 
            // render scene
            setShader(pShaderWorld);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            configDirLightShadowMap();
            if (ptCubemap)
                configPtLightCubemap(ptCubeFarPlane);
            else
                configPtLightShadowMap(ptNearPlane, ptFarPlane);

            renderScene();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwSwapBuffers(glWindow);
        glfwPollEvents();
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
        glClear(GL_DEPTH_BUFFER_BIT);
        renderScene(false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void World::generatePtShadowMap(float nearPlane, float farPlane)
{
    glm::mat4 lightMtrx = glm::mat4(0.f);
    unsigned int depthMapFBO = 0;
    setShader(pShaderShadow);
    for (int i = 0; i < ptLights.size(); i++) {
        //perspective projection
        lightMtrx = ptLights[i]->createMatrix(nearPlane, farPlane, false);
        pShaderShadow->setMat4("lightSpaceMatrix", lightMtrx);

        depthMapFBO = ptLights[i]->getShadowMapFBO();
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderScene(false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void World::generatePtCubemap(float nearPlane, float farPlane)
{
    unsigned int cubemapFBO = 0;
    unsigned int cubemap = 0;
    std::vector<glm::mat4> shadowMatrices;
    setShader(pShaderCubemap);
    pShaderCubemap->setFloat("farPlane", farPlane);
    for (int i = 0; i < ptLights.size(); i++) {
        cubemapFBO = ptLights[i]->getCubemapFBO();
        glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowMatrices = ptLights[i]->createCubemapMatrix(nearPlane, farPlane);
        for (int face = 0; face < 6; face++) {
            pShaderCubemap->setMat4("shadowMatrices[" + std::to_string(face) + "]", shadowMatrices[face]);
        }
        pShaderCubemap->setVec3("lightPos", ptLights[i]->getPos());

        renderScene();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void World::configDirLightShadowMap()
{
    unsigned int depthMap = 0;
    glm::mat4 lightMtrx = glm::mat4(0.f);
    pShaderWorld->use();
    pShaderWorld->setBool("dirLights.debug", debugDepthMap);
    for (int i = 0; i < dirLights.size(); i++) {
        lightMtrx = dirLights[i]->getMatrix();
        pShaderWorld->setMat4("dirLights.matrics[" + std::to_string(i) + "]", lightMtrx);
        pShaderWorld->setVec3("dirLights.direction[" + std::to_string(i) + "]", dirLights[i]->getPos());
        pShaderWorld->setVec3("dirLights.color[" + std::to_string(i) + "]", dirLights[i]->getColor()*dirLights[i]->getStrength());

        depthMap = dirLights[i]->getShadowMap();
        pShaderWorld->setInt("dirLights.shadowMap" + std::to_string(i), 3 + i);
        glActiveTexture(GL_TEXTURE3 + i);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }
}

void World::configPtLightShadowMap(float nearPlane, float farPlane)
{
    unsigned int depthMap = 0;
    glm::mat4 lightMtrx = glm::mat4(0.f);
    pShaderWorld->use();
    pShaderWorld->setFloat("ptLights.nearPlane", nearPlane);
    pShaderWorld->setFloat("ptLights.farPlane", farPlane);
    pShaderWorld->setBool("ptLights.debug", debugDepthMap);
    for (int i = 0; i < ptLights.size(); i++) {
        lightMtrx = ptLights[i]->getMatrix();
        pShaderWorld->setMat4("ptLights.matrics[" + std::to_string(i) + "]", lightMtrx);
        pShaderWorld->setVec3("ptLights.direction[" + std::to_string(i) + "]", ptLights[i]->getPos());
        pShaderWorld->setVec3("ptLights.color[" + std::to_string(i) + "]", ptLights[i]->getColor() * ptLights[i]->getStrength());

        depthMap = ptLights[i]->getShadowMap();
        pShaderWorld->setInt("ptLights.shadowMap" + std::to_string(i), 5 + i);
        glActiveTexture(GL_TEXTURE5 + i);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }
}

void World::configPtLightCubemap(float farPlane)
{
    unsigned int cubemap = 0;
    pShaderWorld->use();
    pShaderWorld->setFloat("ptLights.farPlane", farPlane);
    pShaderWorld->setBool("ptLights.debug", debugDepthMap);
    for (int i = 0; i < ptLights.size(); i++) {
        pShaderWorld->setVec3("ptLights.position[" + std::to_string(i) + "]", ptLights[i]->getPos());
        pShaderWorld->setVec3("ptLights.color[" + std::to_string(i) + "]", ptLights[i]->getColor()*ptLights[i]->getStrength());

        cubemap = ptLights[i]->getCubemap();
        pShaderWorld->setInt("ptLights.cubeMap" + std::to_string(i), 7 + i);
        glActiveTexture(GL_TEXTURE7 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    }
}

void World::renderShadowMap()
{
    if (pCtrlLight) {
        // ortho {true: linear, false: nonlinear}
        bool ortho = pCtrlLight->isOrthoProjection();
        unsigned int depthMap = pCtrlLight->getShadowMap();
        float nearPlane = pCtrlLight->getProjNearPlane();
        float farPlane = pCtrlLight->getProjFarPlane();

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

    pFloor->setShader(pShaderObj);
    pCube->setShader(pShaderObj);
    pCow->setShader(pShaderObj);
    pRobot->setShader(pShaderObj);
    if (pBird)
        pBird->setShader(pShaderObj);
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
    
    pFloor->render();
    pCow->render();
    pRobot->render();
    pCube->render();
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

    if (glfwGetKey(glWindow, GLFW_KEY_Z) == GLFW_PRESS) {
        showDepthMap = true;
    }
    else {
        showDepthMap = false;
    }

    if (glfwGetKey(glWindow, GLFW_KEY_X) == GLFW_PRESS) {
        debugDepthMap = true;
    }
    else {
        debugDepthMap = false;
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