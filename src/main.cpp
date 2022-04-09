#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <rg/Error.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

unsigned int loadTexture(char const * path);

unsigned int loadCubemap(vector<std::string> faces);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int getRandNumber(int min,int max);

void setUpShader(Shader shader,glm::vec3 position,glm::vec3 specular,glm::vec3 diffuse,glm::vec3 ambient,float constant,float linear,float quadratic,glm::mat4 projection,glm::mat4 view,glm::vec3 camPosition,bool point_spot,float cutOff,float outerCutoff,glm::vec3 direction);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
bool hdr=false;
bool invert=false;
bool FlashLight=true;
bool greyScale=false;
float exposure=1.0f;
bool inShattle=true;
glm::vec3 shattlePosition;


// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera (glm::vec3(0.0f, 35.0f, 0.0f));
bool CameraMouseMovementUpdateEnabled = true;

glm::vec3 dif;
glm::vec3 spec;

glm::vec3 ambientSpot=glm::vec3(0.0f);
glm::vec3 diffuseSpot=dif;
glm::vec3 specularSpot=spec;
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};



struct PlanetsInfo{
    glm::vec3 sunPosition = glm::vec3(0.0f);
    float sunScale = 2.0f;
    float sunRotationSpeed=0.03;

    float earthScale=0.6f;
    glm::vec3 earthPosition=glm::vec3(55,0,0);
    float earthRotationSpeed=0.04;
    float earthDistance=earthPosition.x;

    float moonScale=0.2f;
    float moonRotationSpeed=0.3;


    float SaturnScale=1.35f;
    glm::vec3 SaturnPositon=glm::vec3(100.0f,0.0f,0.0f);
    float SaturnRotationSpeed=0.01;
    float SaturnDistance=SaturnPositon.x;

    int numberOfAsteroids=200;
    float radius=20.0f;
    int offset=3;
    int yOffset=1;

};

void processInput(GLFWwindow *window,PlanetsInfo Info);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded textureColorBuffer's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);


    PlanetsInfo Info;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    //Load shaders---------------------------------------
    Shader sunShader("resources/shaders/Sun.vs", "resources/shaders/Sun.fs");
    Shader planetShader("resources/shaders/planetShader.vs", "resources/shaders/planetShader.fs");
    Shader skyboxShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");
    Shader rockShader("resources/shaders/Rocks.vs","resources/shaders/Rocks.fs");
    Shader hdrShader("resources/shaders/hdr.vs","resources/shaders/hdr.fs");
    Shader cubeShattleShader("resources/shaders/cubeShattle.vs","resources/shaders/cubeShattle.fs");

    float rockVertices[] ={
            //front
            -0.5f,-0.5f,0.5f, 0.0f,0.0f,0.0f,0.25f,1.0f,
            0.5f,-0.5f,0.5f, 1.0f,0.0f,0.0f,0.25f,1.0f,
            0.0f,0.5f,0.25f, 0.5f,1.0f,0.0f,0.25f,1.0f,

            //right
            0.5f,-0.5f,0.5f, 1.0f,0.0f,1.0f,0.375f,-0.5f,
            0.0f,-0.5f,-0.5f, 0.5f,0.3f,1.0f,0.375f,-0.5f,
            0.0f,0.5f,0.25f, 0.5f,1.0f,1.0f,0.375f,-0.5f,


            //left
            0.0f,-0.5f,-0.5f, 0.5f,0.3f,-1.0f,-0.375f,-0.5f,
            -0.5f,-0.5f,0.5f, 0.0f,0.0f,-1.0f,-0.375f,-0.5f,
            0.0f,0.5f,0.25f, 0.5f,1.0f,-1.0f,-0.375f,-0.5f,

            //bottom
            -0.5f,-0.5f,0.5f, 0.0f,0.0f,0.0f,-1.0f,0.0f,
            0.0f,-0.5f,-0.5f, 0.5f,0.3f,0.0f,-1.0f,0.0f,
            0.5f,-0.5f,0.5f, 1.0f,0.0f,0.0f,-1.0f,0.0f
    };
    unsigned int indices[] = {
            0,1,2,
            3,4,5,
            6,7,8,
            9,10,11
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rockVertices), rockVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    unsigned int rockTexDiffuse = loadTexture("resources/textures/rock/tileable1b.png");
    unsigned int rockTexSpecular = loadTexture("resources/textures/rock/tileable1c.png");


    float cubeShattleVertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,


            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };


    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeShattleVertices), cubeShattleVertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int cubeTexture= loadTexture("resources/textures/glass/3.jpg");

    //Hdr framebuffer(used for other effects also)--------------------------------------

    unsigned int hdrFBO;
    glGenFramebuffers(1,&hdrFBO);
    unsigned int collorBuffer;
    glGenTextures(1,&collorBuffer);
    glBindTexture(GL_TEXTURE_2D,collorBuffer);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,SCR_WIDTH,SCR_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    unsigned int rboDepth;
    glGenRenderbuffers(1,&rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER,rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,SCR_WIDTH,SCR_HEIGHT);


    glBindFramebuffer(GL_FRAMEBUFFER,hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,collorBuffer,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,rboDepth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        ASSERT(false,"Framebuffer not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);



    float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    unsigned int quadVAO,quadVBO;


    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    float skyboxVertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };

    //configure skybox--------------------------------------------------

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

    std::vector<std::string> faces{
            "resources/textures/space/bkg1_right.png",
            "resources/textures/space/bkg1_left.png",
            "resources/textures/space/bkg1_top.png",
            "resources/textures/space/bkg1_bot.png",
            "resources/textures/space/bkg1_front.png",
            "resources/textures/space/bkg1_back.png"
    };


    unsigned int cubemapTexture=loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox",0);

    rockShader.use();
    rockShader.setInt("diffuseMap",0);
    rockShader.setInt("specularMap",1);

    cubeShattleShader.use();
    cubeShattleShader.setInt("diffuse",0);

    // load models----------------------------------------------

    Model sunModel("resources/objects/Sun/Sun.obj");
    Model earthModel("resources/objects/earth2/Earth 2K.obj");
    Model moonModel("resources/objects/moon/Moon 2K.obj");
    Model SaturnModel("resources/objects/Saturn/Saturn.obj");


    //Light init--------------------------------------------------
    PointLight pointLight;
    pointLight.position = glm::vec3(0.0f,0.0f,0.0f);
    pointLight.ambient = glm::vec3(0.8);
    pointLight.diffuse = glm::vec3(6.0f);
    pointLight.specular = glm::vec3(2);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    SpotLight spotLight;
    spotLight.position=glm::vec3(0.0f);
    spotLight.direction=glm::vec3(0.0f);
    spotLight.cutOff=glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff=glm::cos(glm::radians(15.0f));
    spotLight.constant=1.0f;
    spotLight.linear=0.09f;
    spotLight.quadratic=0.032;
    spotLight.ambient=ambientSpot;
    spotLight.diffuse=diffuseSpot;
    spotLight.specular=specularSpot;


    std::vector<float>radiusWithOffset(Info.numberOfAsteroids);
    std::vector<float>yDisplacement(Info.numberOfAsteroids);
    std::vector<float>angle(Info.numberOfAsteroids);



    for(int i=0;i<Info.numberOfAsteroids;i++){
        int randNumber=getRandNumber(-Info.offset,Info.offset);
        int randNumber2= getRandNumber(-Info.yOffset,Info.yOffset);
        radiusWithOffset[i]=Info.radius + (float)randNumber;
        yDisplacement[i]=(float)randNumber2;
        angle[i]=glm::radians(glfwGetTime() * getRandNumber(0,100)*0.1);
    }


    srand(glfwGetTime());

    // render loop ---------------------

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        processInput(window,Info);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glBindFramebuffer(GL_FRAMEBUFFER,hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),(float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 300.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        spotLight.position=camera.Position;
        spotLight.direction=camera.Front;
        spotLight.ambient=ambientSpot;
        spotLight.diffuse=dif;
        spotLight.specular=spec;
        //setup Shaders------------------------------------

        setUpShader(planetShader,pointLight.position,pointLight.specular,pointLight.diffuse,pointLight.ambient,pointLight.constant,pointLight.linear,pointLight.quadratic,projection,view,camera.Position,true,spotLight.cutOff,spotLight.outerCutOff,spotLight.direction);
        setUpShader(planetShader,spotLight.position,spotLight.specular,spotLight.diffuse,spotLight.ambient,spotLight.constant,spotLight.linear,spotLight.quadratic,projection,view,camera.Position,false,spotLight.cutOff,spotLight.outerCutOff,spotLight.direction);

        setUpShader(rockShader,pointLight.position,pointLight.specular,pointLight.diffuse,pointLight.ambient,pointLight.constant,pointLight.linear,pointLight.quadratic,projection,view,camera.Position,true,spotLight.cutOff,spotLight.outerCutOff,spotLight.direction);
        setUpShader(rockShader,spotLight.position,spotLight.specular,spotLight.diffuse,spotLight.ambient,spotLight.constant,spotLight.linear,spotLight.quadratic,projection,view,camera.Position,false,spotLight.cutOff,spotLight.outerCutOff,spotLight.direction);


        rockShader.setInt("FlashLight",FlashLight);
        planetShader.setInt("FlashLight",FlashLight);

        cubeShattleShader.use();
        cubeShattleShader.setMat4("projection", projection);
        cubeShattleShader.setMat4("view", view);
        cubeShattleShader.setVec3("viewPosition", camera.Position);


        //SkyBox----------------------------------
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view",glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection",projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
        glDrawArrays(GL_TRIANGLES,0,36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);


        // render sun--------------------------------------------
        float time=(float)glfwGetTime();
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);
        model= glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(Info.sunScale));
        model = glm::rotate(model,Info.sunRotationSpeed*time,glm::vec3(0.0,-1.0,0.0));
        sunShader.setMat4("model", model);
        sunModel.Draw(sunShader);

        //render Earth-----------------------------------------------
        planetShader.use();
        Info.earthPosition=glm::vec3(Info.earthDistance*cos(time * Info.earthRotationSpeed),0,Info.earthDistance*sin(time * Info.earthRotationSpeed));


        model = glm::mat4(1.0f);
        model=glm::translate(model,Info.earthPosition);
        model=glm::scale(model,glm::vec3(Info.earthScale));
        model=glm::rotate(model,float(time*0.5),glm::vec3(0.0,1,0.0));
        planetShader.setMat4("model", model);
        earthModel.Draw(planetShader);



        //render Moon-----------------------------------------
        model=glm::mat4(1.0f);
        model=glm::translate(model,Info.earthPosition);
        model=glm::translate(model,glm::vec3(3*cos(time),0,3*sin(time)));
        model=glm::scale(model,glm::vec3(Info.moonScale));
        model=glm::rotate(model,float(time*0.7),glm::vec3(0.0,1.0,0.0));
        planetShader.setMat4("model", model);
        moonModel.Draw(planetShader);

        //render Saturn--------------------------------------------

        Info.SaturnPositon=glm::vec3(Info.SaturnDistance*cos(time * Info.SaturnRotationSpeed),0,Info.SaturnDistance*sin(time * Info.SaturnRotationSpeed));

        model=glm::mat4(1.0f);
        model=glm::translate(model,Info.SaturnPositon);
        model=glm::scale(model,glm::vec3(Info.SaturnScale));
        model=glm::rotate(model,float(0.1*time),glm::vec3(0.0,1.0,0.0));
        planetShader.setMat4("model", model);
        SaturnModel.Draw(planetShader);


        //Enable culling so asteroid inner sides dont render-------------------------

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        //rocks ------------------------------------
        rockShader.use();
        for(int i=0;i<Info.numberOfAsteroids;i++){
            model=glm::mat4(1.0f);
            float x=radiusWithOffset[i]*cos(glfwGetTime()*0.1*(glm::radians(360.f)/Info.numberOfAsteroids)*i);
            float y= yDisplacement[i];
            float z = radiusWithOffset[i]*sin(glfwGetTime()*0.1*(glm::radians(360.f)/Info.numberOfAsteroids)*i);
            model=glm::translate(model,Info.SaturnPositon);
            model=glm::translate(model,glm::vec3(x,y,z));
            model=glm::scale(model,glm::vec3(0.8f));
            model = glm::rotate(model,(float)glfwGetTime()*angle[i], glm::vec3(0.4f, 0.6f,0.8f));
            rockShader.setMat4("model",model);
            glBindVertexArray(VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,rockTexDiffuse);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D,rockTexSpecular);
            glDrawElements(GL_TRIANGLES,12,GL_UNSIGNED_INT,0);
            glBindVertexArray(0);
        }
        glDisable(GL_CULL_FACE);


        //cubeShattle that's transparent only from inside---------------------------

        glEnable(GL_CULL_FACE);
        for(int i=0;i<2;i++){
            if(i)
                glCullFace(GL_FRONT);
            else
                glCullFace(GL_BACK);
            cubeShattleShader.use();
            cubeShattleShader.setInt("i",i);
            model=glm::mat4(1.0f);
            if(inShattle){
                shattlePosition=camera.Position;
                model=glm::translate(model,camera.Position);
            }
            else{
                model=glm::translate(model,shattlePosition);
            }
            cubeShattleShader.setMat4("model",model);
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,cubeTexture);
            glDrawArrays(GL_TRIANGLES,0,36);
            glBindVertexArray(0);
        }
        glDisable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,collorBuffer);
        hdrShader.setInt("hdr",hdr);
        hdrShader.setFloat("exposure",exposure);
        hdrShader.setInt("invert",invert);
        hdrShader.setInt("greyScale",greyScale);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window,PlanetsInfo Info) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window,GLFW_KEY_F1) == GLFW_PRESS){
        camera.Position=glm::vec3(Info.earthPosition - glm::vec3(0,0,5));
        camera.Front=Info.earthPosition-camera.Position;
    }
    if (glfwGetKey(window,GLFW_KEY_F2) == GLFW_PRESS){
        camera.Position=glm::vec3(Info.earthPosition + glm::vec3(0,10,0));
        camera.Front=Info.earthPosition-camera.Position;
    }

    if (glfwGetKey(window,GLFW_KEY_F3) == GLFW_PRESS){
        camera.Position=glm::vec3(Info.SaturnPositon - glm::vec3(0,0,30));
        camera.Front=Info.SaturnPositon-camera.Position;
    }
    if (glfwGetKey(window,GLFW_KEY_F4) == GLFW_PRESS){
        camera.Position=glm::vec3(Info.SaturnPositon + glm::vec3(0,40,0));
        camera.Front=Info.SaturnPositon-camera.Position;
    }

    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS){
        camera.MovementSpeed=15.0f;
    }
    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT)==GLFW_RELEASE){
        camera.MovementSpeed=2.5f;
    }


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (CameraMouseMovementUpdateEnabled)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key==GLFW_KEY_1 && action==GLFW_PRESS) {
        invert=false;
        greyScale= false;
        hdr = !hdr;
    }
    if(key==GLFW_KEY_2 && action==GLFW_PRESS){
        hdr = false;
        greyScale= false;
        invert=!invert;
    }
    if(key==GLFW_KEY_3 && action==GLFW_PRESS){
        hdr= false;
        invert= false;
        greyScale=!greyScale;
    }
    if(key==GLFW_KEY_ENTER && action==GLFW_PRESS){
        inShattle=!inShattle;
    }

    if(key==GLFW_KEY_Q && action==GLFW_PRESS){
        if(exposure>0.0f)
            exposure-=0.03f;
        else
            exposure=0.0f;
    }
    if(key==GLFW_KEY_E && action==GLFW_PRESS){
        exposure+=0.03;
    }
    if(key==GLFW_KEY_F && action==GLFW_PRESS){
        if(!FlashLight){
            dif=glm::vec3(0);
            spec=glm::vec3(0);
            FlashLight=!FlashLight;
        }
        else{
            dif=glm::vec3(1);
            spec=glm::vec3(1);
            FlashLight=!FlashLight;
        }
    }
}
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
int getRandNumber(int min,int max){
    return (min + (rand()%(max-min+1)));
}


void setUpShader(Shader shader,glm::vec3 position,glm::vec3 specular,glm::vec3 diffuse,glm::vec3 ambient,float constant,float linear,float quadratic,glm::mat4 projection,glm::mat4 view,glm::vec3 camPosition,bool point_spot,float cutOff,float outerCutoff,glm::vec3 direction){
    if(point_spot){
        shader.use();
        shader.setVec3("pointLight.position", position);
        shader.setVec3("pointLight.specular", specular);
        shader.setVec3("pointLight.diffuse", diffuse);
        shader.setVec3("pointLight.ambient", ambient);
        shader.setFloat("pointLight.constant", constant);
        shader.setFloat("pointLight.linear", linear);
        shader.setFloat("pointLight.quadratic", quadratic);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPosition", camPosition);
    }
    else{
        shader.use();
        shader.setVec3("spotLight.position", position);
        shader.setVec3("spotLight.direction",direction);
        shader.setVec3("spotLight.specular", specular);
        shader.setVec3("spotLight.diffuse", diffuse);
        shader.setVec3("spotLight.ambient", ambient);
        shader.setFloat("spotLight.constant", constant);
        shader.setFloat("spotLight.linear", linear);
        shader.setFloat("spotLight.quadratic", quadratic);
        shader.setFloat("cutOff",cutOff);
        shader.setFloat("outerCutOff",outerCutoff);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPosition", camPosition);
    }

}
