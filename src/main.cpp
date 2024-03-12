#include <cstdlib>
#include <cmath>
#include <iostream>
#include <array>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Shader.hpp>
#include <Camera.hpp>
#include <Model.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow*, int, int);
void process_input(GLFWwindow*);
void mouse_callback(GLFWwindow*, double, double);
void scroll_callback(GLFWwindow* , double, double);
unsigned int loadTexture(const char* const path);
unsigned int loadCubeMap(const std::array<std::string, 6>& faces);

constexpr int windowWidth = 1920;
constexpr int windowHeight = 1080;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = windowWidth / 2.f;
float lastY = windowHeight / 2.f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

int main() {
    std::ios::sync_with_stdio(false);

    stbi_set_flip_vertically_on_load(false);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto *window = glfwCreateWindow(windowWidth, windowHeight, "Unloved", nullptr, nullptr);
    if(!window) {
        std::cout << "Couldn't create window!\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD.\n";
        return EXIT_FAILURE;
    }

    // NOTE: need to click on the inside the window first to the mouse doesn't go out of bounds.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glViewport(0, 0, windowWidth, windowHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    Shader shader("./shaders/depthTesting.vs", "./shaders/depthTesting.fs");
    // Shader shaderSingleColour("./shaders/shaderSingleColour.vs", "./shaders/shaderSingleColour.fs");
    // Shader shaderGrass("./shaders/shaderGrass.vs", "./shaders/shaderGrass.fs");
    // Shader shaderWindow("./shaders/shaderWindow.vs", "./shaders/shaderWindow.fs");
    Shader shaderContainer("./shaders/containerShader.vs", "./shaders/containerShader.fs");
    Shader shaderSkybox("./shaders/skyboxShader.vs", "./shaders/skyboxShader.fs");

    constexpr std::array<float, 200> cubeVertices{
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
        // Top face (x)
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
    };

    // constexpr std::array<glm::vec3, 5> vegetation{
    //     glm::vec3(-1.5f, 0.f, -0.48f),
    //     glm::vec3( 1.5f, 0.f,  0.51f),
    //     glm::vec3( 0.0f, 0.f,  0.7f),
    //     glm::vec3(-0.3f, 0.f, -2.3f),
    //     glm::vec3( 0.5f, 0.f, -0.6f)
    // };

    // constexpr std::array<glm::vec3, 5> windows{
    //     glm::vec3( 0.0f, 0.0f, 0.74f),
    //     glm::vec3(-1.5f, 0.0f, -0.45f),
    //     glm::vec3( 1.5f, 0.0f, 0.54f),
    //     glm::vec3(-0.3f, 0.0f, -2.2f),
    //     glm::vec3( 0.5f, 0.0f, -0.4f),
    // };

    constexpr std::array<float, 30> planeVertices{
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };

    // constexpr std::array<float, 30> transparentVertices{
    //     // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
    //     0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
    //     0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
    //     1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

    //     0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
    //     1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
    //     1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    // };

    unsigned int cubeVAO{ 0 }, cubeVBO{ 0 };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int planeVAO{ 0 }, planeVBO{ 0 };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);

    // unsigned int grassVAO{ 0 }, grassVBO{ 0 };
    // glGenVertexArrays(1, &grassVAO);
    // glGenBuffers(1, &grassVBO);
    // glBindVertexArray(grassVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    // glBindVertexArray(0);

    // unsigned int windowVAO{ 0 }, windowVBO{ 0 };
    // glGenVertexArrays(1, &windowVAO);
    // glGenBuffers(1, &windowVBO);
    // glBindVertexArray(windowVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    // glBindVertexArray(0);

    //  constexpr std::array<float, 24> quadVertices{
    //     // positions   // texCoords
    //     -0.2f,  1.0f,  0.0f, 0.0f, // top-left
    //     -0.2f,  0.8f,  0.0f, 1.0f, // bottom-left
    //      0.2f,  0.8f,  1.0f, 1.0f, // bottom-right

    //     -0.2f,  1.0f,  0.0f, 0.0f, // top-left
    //      0.2f,  0.8f,  1.0f, 1.0f, // bottom-right
    //      0.2f,  1.0f,  1.0f, 0.0f, // top-right
    // };

    // We've got to change y's coordinates of the texture because when we apply the texture from the framebuffer,
    // it does it upside down.
    constexpr std::array<float, 24> quadVertices{
        // positions   // texCoords
        -0.4f,  1.0f,  0.0f, 1.0f, // top-left
        -0.4f,  0.6f,  0.0f, 0.0f, // bottom-left
         0.4f,  0.6f,  1.0f, 0.0f, // bottom-right

        -0.4f,  1.0f,  0.0f, 1.0f, // top-left
         0.4f,  0.6f,  1.0f, 0.0f, // bottom-right
         0.4f,  1.0f,  1.0f, 1.0f, // top-right
    };

    constexpr std::array<float, 120> skyboxVertices{
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    unsigned int containerVAO{ 0 }, containerVBO{ 0 };
    glGenVertexArrays(1, &containerVAO);
    glGenBuffers(1, &containerVBO);
    glBindVertexArray(containerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, containerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int skyboxVAO{ 0 }, skyboxVBO{ 0 };
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    unsigned int framebuffer{ 0 };
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    unsigned int textureColourBuffer{ 0 };
    glGenTextures(1, &textureColourBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColourBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBuffer, 0);

    unsigned int rbo{ 0 };
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete, bro.\n";
        return EXIT_FAILURE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const std::array<std::string, 6> faces{
        "./assets/skybox/right.jpg",
        "./assets/skybox/left.jpg",
        "./assets/skybox/top.jpg",
        "./assets/skybox/bottom.jpg",
        "./assets/skybox/front.jpg",
        "./assets/skybox/back.jpg"
    };

    // unsigned int cubeTexture = loadTexture("./assets/marble.jpg");
    unsigned int floorTexture = loadTexture("./assets/metal.png");
    // unsigned int grassTexture = loadTexture("./assets/grass.png");
    // unsigned int windowTexture = loadTexture("./assets/blending_transparent_window.png");
    unsigned int containerTexture = loadTexture("./assets/haruhi.jpg");
    unsigned int cubemapTexture = loadCubeMap(faces);

    shader.use();
    shader.setUniformInt("texture1", 0);

    shaderContainer.use();
    shaderContainer.setUniformInt("texture1", 0);

    while(!glfwWindowShouldClose(window)) {
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // glEnable(GL_DEPTH_TEST);

        // Mirrored world.
        // glClearColor(.1f, .1f, .1f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shader.use();

        // glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), .1f, 100.f);
        // glm::mat4 model = glm::mat4(1.0f);

        // // Mirrored-effect
        // camera.pitch = -camera.pitch;
        // camera.yaw += 180.0f;
        // camera.processMouseMovement(0, 0, false);
        // glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix())); //camera.getViewMatrix();
        // camera.yaw -= 180.0f;
        // camera.pitch = -camera.pitch;
        // camera.processMouseMovement(0, 0, true);

        // // skybox
        // glDepthMask(GL_FALSE); // Disable writing to the depth buffer.
        // shaderSkybox.use();
        // glBindVertexArray(skyboxVAO);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        // shaderSkybox.setMat4("projection", projection);
        // shaderSkybox.setMat4("view", view);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glBindVertexArray(0);
        // glDepthMask(GL_TRUE); // Leave it as it was.

        // // floor
        // glBindVertexArray(planeVAO);
        // glBindTexture(GL_TEXTURE_2D, floorTexture);
        // shader.setMat4("view", view);
        // shader.setMat4("projection", projection);
        // shader.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        // glBindVertexArray(0);

        // // cubes
        // glBindVertexArray(cubeVAO);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, containerTexture);
        // model = glm::translate(model, glm::vec3(-1.f, .02f, -1.f));
        // shader.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // model = glm::mat4(1.f);
        // model = glm::translate(model, glm::vec3(2.f, .02f, 0.f));
        // shader.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glBindVertexArray(0);

        // Once we get here, we have the mirrored world stored in the texture.
        // rear-view mirror.
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glClearColor(1.f, 1.f, 1.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), .1f, 100.f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.getViewMatrix();

        // model = glm::mat4(1.0f);
        // // view = camera.getViewMatrix();
        // view = glm::mat4(glm::mat3(camera.getViewMatrix())); //camera.getViewMatrix();

        // glBindVertexArray(planeVAO);
        // glBindTexture(GL_TEXTURE_2D, floorTexture);
        // shader.setMat4("view", view);
        // shader.setMat4("projection", projection);
        // shader.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        // glBindVertexArray(0);

        shader.use();
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        model = glm::translate(model, glm::vec3(-1.f, .02f, -1.f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(2.f, .02f, 0.f));
        shader.setMat4("model", model);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        view = glm::mat4(glm::mat3(camera.getViewMatrix()));
        // skybox
        glDepthMask(GL_FALSE); // Disable writing to the depth buffer.
        shaderSkybox.use();
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        shaderSkybox.setMat4("projection", projection);
        shaderSkybox.setMat4("view", view);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE); // Leave it as it was.

        // shaderContainer.use();
        // glBindVertexArray(containerVAO);
        // glBindTexture(GL_TEXTURE_2D, textureColourBuffer);
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        // Render everything we computed.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}

void framebuffer_size_callback([[maybe_unused]]GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::FORWARD, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::BACKWARD, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::RIGHT, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::LEFT, deltaTime);
    }
}

void mouse_callback([[maybe_unused]]GLFWwindow* window, double mouseXPos, double mouseYPos) {
    if(firstMouse) {
        lastX = static_cast<float>(mouseXPos);
        lastY = static_cast<float>(mouseYPos);
        firstMouse = false;
    }

    const float xOffset = static_cast<float>(mouseXPos) - lastX;
    const float yOffset = lastY - static_cast<float>(mouseYPos);
    lastX = static_cast<float>(mouseXPos);
    lastY = static_cast<float>(mouseYPos);

    camera.processMouseMovement(xOffset, yOffset, true);
}

void scroll_callback([[maybe_unused]]GLFWwindow* window, [[maybe_unused]]double xoffset, double yoffset)
{
    camera.processMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const char* const path) {
    unsigned int textureID{ 0 };
    glGenTextures(1, &textureID);

    int width{ 0 }, height{ 0 }, nrComponents{ 0 };
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if(!data) {
        std::cerr << "::" << __func__ << "::ERROR: Couldn't load texture: " << path << '\n';
        stbi_image_free(data);
        return textureID;
    }

    GLenum format = GL_RED;
    if(nrComponents == 1) {
        format = GL_RED;
    } else if(nrComponents == 3) {
        format = GL_RGB;
    } else if(nrComponents == 4) {
        format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if(nrComponents == 4) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

unsigned int loadCubeMap(const std::array<std::string, 6>& faces) {
    unsigned int textureID{ 0 };
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width{ 0 }, height{ 0 }, nrComponents{ 0 };
    for(unsigned int i{ 0 }; i < faces.size(); ++i) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if(!data) {
            std::cerr << "Failed to load cubeMap, bro.\n";
            stbi_image_free(data);
            return 0;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
