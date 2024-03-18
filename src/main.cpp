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

constexpr int windowWidth{ 1920 };
constexpr int windowHeight{ 1080 };

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = windowWidth / 2.f;
float lastY = windowHeight / 2.f;

bool blinn{ false };
bool blinnKeyPressed{ false };

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glViewport(0, 0, windowWidth, windowHeight);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    unsigned int depthMapFBO{ 0 };
    glGenFramebuffers(1, &depthMapFBO);

    // TODO: try to change this values to see what happens
    constexpr unsigned int shadowWidth{ 1024 }, shadowHeight{ 1024 };
    unsigned int depthMap{ 0 };
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    constexpr std::array<float, 4> borderColour{ 1.f, 1.f, 1.f, 1.f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColour[0]);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // To complete the framebuffer.
    glReadBuffer(GL_NONE); // To complete the framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    constexpr std::array<float, 320> vertices{
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

    unsigned int cubeVAO{ 0 }, cubeVBO{ 0 };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    constexpr std::array<float, 48> planeVertices{
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

    unsigned int planeVAO{ 0 }, planeVBO{ 0 };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
    glBindVertexArray(planeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader lightShader("./shaders/lightShader.vs", "./shaders/lightShader.fs");
    Shader simpleDepthShader("./shaders/simpleDepthShader.vs", "./shaders/simpleDepthShader.fs");

    constexpr glm::vec3 lightPosition(-2.f, 4.f, -1.f);
    constexpr float orthoNearPlane{ 1.f }, orthoFarPlane{ 7.5f };
    const glm::mat4 lightView{ glm::lookAt(lightPosition,
                                           glm::vec3(0.f),
                                           glm::vec3( 0.f, 1.f,  0.f)) };
    const glm::mat4 lightProjection{ glm::ortho(-10.f, 10.f, -10.f, 10.f, orthoNearPlane, orthoFarPlane) };
    const glm::mat4 lightSpaceMatrix{ lightProjection * lightView };

    const unsigned int woodTexture{ loadTexture("./assets/wood.png") };

    lightShader.use();
    lightShader.setUniformInt("diffuseTexture", 0);
    lightShader.setUniformInt("shadowMap", 1);

    // glEnable(GL_CULL_FACE);

    while(!glfwWindowShouldClose(window)) {
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glCullFace(GL_BACK);
        // 1. First render to the depth map.
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glViewport(0, 0, shadowWidth, shadowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glm::mat4 model{ glm::mat4(1.f) };
        // Floor.
        simpleDepthShader.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        // glCullFace(GL_FRONT);
        // First cube.
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(0.f, 1.5f, 0.f));
        model = glm::scale(model, glm::vec3(.5f));
        simpleDepthShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // Second cube.
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(2.f, 0.f, 1.f));
        model = glm::scale(model, glm::vec3(.5f));
        simpleDepthShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // Third cube.
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-1.f, 0.f, 2.f));
        model = glm::rotate(model, glm::radians(60.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)));
        model = glm::scale(model, glm::vec3(.25f));
        simpleDepthShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Render scene as normal with shadow mapping (using depth map).
        // const glm::mat4 projection{ glm::perspective(glm::radians(camera.zoom), static_cast<float>(windowWidth) / windowHeight, .1f, 100.f) };
        // const glm::mat4 view{ camera.getViewMatrix() };
        // glm::mat4 model{ glm::mat4(1.f) };
        // glCullFace(GL_BACK);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection{ glm::perspective(glm::radians(camera.zoom), static_cast<float>(windowWidth) / windowHeight, .1f, 100.f) };
        glm::mat4 view{ camera.getViewMatrix() };

        lightShader.use();
        lightShader.setVec3("lightPosition", lightPosition);
        lightShader.setVec3("viewerPosition", camera.position);
        lightShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        // Floor.
        model = glm::mat4(1.f);
        lightShader.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        // First cube.
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(0.f, 1.5f, 0.f));
        model = glm::scale(model, glm::vec3(.5f));
        lightShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // Second cube.
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(2.f, 0.f, 1.f));
        model = glm::scale(model, glm::vec3(.5f));
        lightShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // Third cube.
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-1.f, 0.f, 2.f));
        model = glm::rotate(model, glm::radians(60.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)));
        model = glm::scale(model, glm::vec3(.25f));
        lightShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

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
    } else if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::DOWN, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::UP, deltaTime);
    }

    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !blinnKeyPressed) {
        blinn = (blinn) ? false : true;
        blinnKeyPressed = true;
    }
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        blinnKeyPressed = false;
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
    GLenum format2 = GL_RGB;
    if(nrComponents == 1) {
        format = GL_RED;
    } else if(nrComponents == 3) {
        // format = GL_SRGB;
        format = GL_RGB;
    } else if(nrComponents == 4) {
        // format = GL_SRGB_ALPHA;
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
