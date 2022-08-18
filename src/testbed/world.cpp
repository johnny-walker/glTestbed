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
    if (pFirst) delete pFirst;

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
    if (mouseCallback) {
        glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(glWindow, mouse_callback);
        glfwSetScrollCallback(glWindow, scroll_callback);
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_CULL_FACE);  //skybox must disable this
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // init shaders
    pShaderPBR = new Shader("pbr_model.vs", "pbr_model.fs");
    pShaderShadow = new Shader("shadow_mapping.vs", "shadow_mapping.fs");
    pShaderCubemap = new Shader("cubemap_depth.vs", "cubemap_depth.fs", "cubemap_depth.gs");
    // IBL spacular shaders
    pShaderEnvCubemap = new Shader("env_cubemap.vs", "equi_to_cubemap.fs");
    pShaderIrrConv = new Shader("env_cubemap.vs", "irr_convolution.fs");
    pShaderPrefltr = new Shader("env_cubemap.vs", "prefilter.fs");
    pShaderBRDF = new Shader("brdf.vs", "brdf.fs");
    pShaderBG = new Shader("background.vs", "background.fs");

    if (!pShaderPBR)
        return false;
    pShaderPBR->use();

    bool pbrModel = true;
    if (pbrModel) {
        initWorld(!showSkybox, !showSkybox);
        initPBRModel();
    } else {
        initWorld(true, true);
        initModel();
    }

    return true;
}

void World::initWorld(bool bFloor, bool bCube)
{
    // init camera
    pCamera = new Camera(glm::vec3(0.f, 1.f, 9.f));

    initDirLights(1);
    initPtLights(1);

    // create floor
    if (bFloor) {
        pFloor = new Floor(scrWidth, scrHeight);
        pFloor->init(pShaderPBR, pCamera);
    }

    if (bCube) {
        pCube = new Cube(scrWidth, scrHeight);
        pCube->init(pShaderPBR, pCamera);
        pCube->setAngle(glm::radians(60.f), 1);
        pCube->setPos(-7.5f, 0.f, -5.5f);
        pCube->setScale(0.5f);
    }
}

// at most 2 for point lights and direction lights
void World::initDirLights(int count)
{
    pShaderPBR->use();

    if (count > 0) {
        DirLight* pDirLight = new DirLight(0, scrWidth, scrHeight);
        pDirLight->init(pShaderPBR, pCamera);
        pDirLight->setPos(3.f, 2.f, 1.5f);
        pDirLight->setPrimaryColor(0);  //white
        pDirLight->setStrength(1.f);
        dirLights.push_back(pDirLight);
    }
    if (count > 1) {
        DirLight* pDirLight = new DirLight(1, scrWidth, scrHeight);
        pDirLight->init(pShaderPBR, pCamera);
        pDirLight->setPos(2.f, 2.f, -1.5f);
        pDirLight->setPrimaryColor(0);  //white
        pDirLight->setStrength(1.f);
        dirLights.push_back(pDirLight);
    }
    pShaderPBR->setInt("dirLights.count", (int)dirLights.size());
}

void World::initPtLights(int count)
{
    pShaderPBR->use();

    if (count > 0) {
        PointLight* pPtLight = new PointLight(0, scrWidth, scrHeight);
        pPtLight->init(pShaderPBR, pCamera);
        pPtLight->setPos(-1.f, 1.2f, 1.5f);
        pPtLight->setPrimaryColor(2);   //orange      
        pPtLight->setStrength(1.f);
        ptLights.push_back(pPtLight);
    }
    if (count > 1) {
        PointLight* pPtLight = new PointLight(1, scrWidth, scrHeight);
        pPtLight->init(pShaderPBR, pCamera);
        pPtLight->setPos(-1.5f, 1.5f, -6.5f);
        pPtLight->setPrimaryColor(4);   //green      
        pPtLight->setStrength(0.5f);
        ptLights.push_back(pPtLight);
    }
    pShaderPBR->setInt("ptLights.count", (int)ptLights.size());

    createPtCubemapTexture();
}

bool World::initPBRModel()
{
    pFirst = new BaseModel(scrWidth, scrHeight, "../../resources/objects/glasses/PF_eyeware.obj");
    //pFirst = new BaseModel(scrWidth, scrHeight, "../../resources/objects/glasses_cat/cat_eyeware.obj");
    //pFirst = new BaseModel(scrWidth, scrHeight, "../../resources/objects/glasses_lace/lace_eyeware.obj");
    
    if (!pFirst)
        return false;

    pFirst->init(pShaderPBR, pCamera);
    pFirst->setAngle(glm::radians(30.f), 1);
    pFirst->setPos(0.f, 3.f, 0.f);
    pFirst->setScale(0.2f);
    pCtrlTarget = pFirst;


    initIBLSpecular("../../resources/hdr/newport_loft.hdr");
    pShaderPBR->use();

    return true;
}

bool World::initModel()
{
    pFirst = new BaseModel(scrWidth, scrHeight, "../../resources/objects/spot/spot.obj");
    
    if (!pFirst)
        return false;

    pFirst->init(pShaderPBR, pCamera);
    pFirst->setAngle(glm::radians(210.f), 1);
    pFirst->setPos(0.f, 0.25f, 0.f);
    pCtrlTarget = pFirst;

    return true;
}

void World::render() 
{
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    pShaderPBR->use();
    glViewport(0, 0, scrWidth, scrHeight);  // restore to screen resolution
    
    while (!glfwWindowShouldClose(glWindow)) {
        // per-frame time dalta
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(deltaTime);

        setShader(pShaderPBR);
        pShaderPBR->setInt("lightingModel", lightModel);
        pShaderPBR->setVec3("viewPos", pCamera->Position);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
        float dirNearPlane = 0.1f, dirFarPlane = 10.f;
        float ptNearPlane = 1.f, ptFarPlane = 25.f;

        pShaderPBR->setInt("calcShadow", (int)calcShadow);
        if (calcShadow) {
            generateDirShadowMap(dirNearPlane, dirFarPlane);
            generatePtCubemap(ptNearPlane, ptFarPlane);
        }

        // render scene
        setShader(pShaderPBR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        configDirLightShadowMap();
        configPtLightShadowMap(ptFarPlane);

        renderSkybox();
        renderScene(true);

        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }
    glDisable(GL_BLEND);
}

void World::generateDirShadowMap(float nearPlane, float farPlane)
{
    if ( !pShaderShadow || dirLights.size() == 0)
        return;

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
    if (dirLights.size() == 0)
        return;

    pShaderPBR->use();
    for (int i = 0; i < dirLights.size(); i++) {
        glm::mat4 lightMtrx = dirLights[i]->getMatrix();
        pShaderPBR->setMat4("dirLights.matrics[" + std::to_string(i) + "]", lightMtrx);
        pShaderPBR->setVec3("dirLights.direction[" + std::to_string(i) + "]", dirLights[i]->getPos());
        pShaderPBR->setVec3("dirLights.color[" + std::to_string(i) + "]", dirLights[i]->getColor() * dirLights[i]->getStrength());

        unsigned int depthMap = dirLights[i]->getShadowMap();
        pShaderPBR->setInt("dirLights.shadowMap" + std::to_string(i), i+6);
        glActiveTexture(GL_TEXTURE6 + i);
        glBindTexture(GL_TEXTURE_2D, depthMap);
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

    float aspect = 1.f; // (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
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
    pShaderPBR->setFloat("ptLights.farPlane", farPlane);
    for (int i = 0; i < ptLights.size(); i++) {
        pShaderPBR->setVec3("ptLights.position[" + std::to_string(i) + "]", ptLights[i]->getPos());
        pShaderPBR->setVec3("ptLights.color[" + std::to_string(i) + "]", ptLights[i]->getColor() * ptLights[i]->getStrength());

        pShaderPBR->setInt("ptLights.cubeMap" + std::to_string(i), i+8);
        glActiveTexture(GL_TEXTURE8 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
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
    if (pFirst) pFirst->setShader(pShaderObj);
}

void World::renderScene(bool drawSphere)
{
    // render point lights sphere 
    pShaderPBR->setInt("lightId", (int)-1);
    if (drawSphere) {
        if (lightModel == 0 || lightModel == 2) {
            for (int i = 0; i < ptLights.size(); i++)
                ptLights[i]->render();
        }
    }
    if (!iblMap || !showSkybox) {
        if (pFloor) pFloor->render();
        if (pCube)  pCube->render();
    }
    if (pFirst) pFirst->render();
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

void World::renderCube()
{
    if (cubeVAO == 0)
    {
        //skybox
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glDisable(GL_CULL_FACE); // if enabled, cant shade clock-wise triangles

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
}

void World::initIBLSpecular(char const* filename)
{
    // load the HDR environment map
    stbi_set_flip_vertically_on_load(true);
    int width = 0, height = 0, nComponents = 0;
    float* data = stbi_loadf(filename, &width, &height, &nComponents, 0);

    if (!data) {
        std::cout << "Failed to load HDR image." << std::endl;
        return;
    }
    iblMap = true;
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    // setup framebuffer
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


    // 1. setup cubemap to render to and attach to framebuffer
    pShaderEnvCubemap->use();

    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set up projection and view matrices for capturing data onto the 6 cubemap face directions
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // convert HDR equirectangular environment map to cubemap 
    pShaderEnvCubemap->setInt("equirectangularMap", 0);
    pShaderEnvCubemap->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        pShaderEnvCubemap->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // 2. create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
    pShaderIrrConv->use();

    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // solve diffuse integral by convolution to create an irradiance (cube)map.
    pShaderIrrConv->setInt("environmentMap", 0);
    pShaderIrrConv->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        pShaderIrrConv->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
    pShaderPrefltr->use();

    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    pShaderPrefltr->setInt("environmentMap", 0);
    pShaderPrefltr->setMat4("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        pShaderPrefltr->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            pShaderPrefltr->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4. generate a 2D LUT from the BRDF equations used.
    pShaderBRDF->use();

    // pre-allocate enough memory for the LUT texture.
    glGenTextures(1, &brdfLUTTextureMap);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTextureMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTextureMap, 0);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    // bind pre-computed IBL data
    pShaderPBR->use();
    pShaderPBR->setInt("envMap", (int) true);
    pShaderPBR->setInt("irradianceMap", 10);
    pShaderPBR->setInt("prefilterMap", 11);
    pShaderPBR->setInt("brdfLUT", 12);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTextureMap);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, scrWidth, scrHeight);  // restore to screen resolution

}

void World::renderSkybox()
{
    if (showSkybox) {
        glm::mat4 projection = glm::perspective(glm::radians(pCamera->Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);

        pShaderBG->use();
        pShaderBG->setMat4("projection", projection);
        pShaderBG->setInt("environmentMap", 0);
        pShaderBG->setMat4("view", pCamera->GetViewMatrix());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
        //glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
        renderCube();

        pShaderPBR->use();  // restore
    }

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
        pCtrlTarget = pFirst;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F2) == GLFW_PRESS) {
        pCtrlTarget = pCube;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F3) == GLFW_PRESS) {
        // todo
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F4) == GLFW_PRESS) {
        // todo
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F5) == GLFW_PRESS) {
        if (dirLights.size() > 0)
            pCtrlTarget = pCtrlLight = dirLights[0];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F6) == GLFW_PRESS) {
        if (dirLights.size() > 1)
            pCtrlTarget = pCtrlLight = dirLights[1];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F7) == GLFW_PRESS) {
        if (ptLights.size() > 0)
            pCtrlTarget = pCtrlLight = ptLights[0];
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F8) == GLFW_PRESS) {
        if (ptLights.size() > 1)
            pCtrlTarget = pCtrlLight = ptLights[1];
    }

    // switch lighting algorithms
    if (glfwGetKey(glWindow, GLFW_KEY_F9) == GLFW_PRESS) {
        // turn on all lights
        lightModel = 0;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F10) == GLFW_PRESS) {
        // turn on direction lights only
        lightModel = 1;
    }
    else if (glfwGetKey(glWindow, GLFW_KEY_F11) == GLFW_PRESS) {
        // turn on point lights only
        lightModel = 2;
    }

    calcShadow = (glfwGetKey(glWindow, GLFW_KEY_Z) == GLFW_PRESS) ? !CALC_SHADOW : CALC_SHADOW;
    showSkybox = (glfwGetKey(glWindow, GLFW_KEY_X) == GLFW_PRESS) ? !SHOW_SKYBOX : SHOW_SKYBOX;

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