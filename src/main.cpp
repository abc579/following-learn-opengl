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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glViewport(0, 0, windowWidth, windowHeight);

    Model backpackModel("./assets/backpack/backpack.obj");

    Shader geometryShader("./shaders/geometryTest.vs", "./shaders/geometryTest.gs", "./shaders/geometryTest.fs");
    Shader normalShader("./shaders/modelLoading.vs", "./shaders/modelLoading.fs");

    constexpr std::array<float, 20> points{
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    unsigned int pointsVAO{ 0 }, pointsVBO{ 0 };
    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(5 * sizeof(float)));
    glBindVertexArray(0);

    while(!glfwWindowShouldClose(window)) {
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        glClearColor(.1f, .1f, .1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), .1f, 100.f);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.f);

        normalShader.use();
        normalShader.setMat4("projection", projection);
        normalShader.setMat4("view", view);
        normalShader.setMat4("model", model);
        normalShader.setVec3("viewerPosition", camera.position);
        normalShader.setVec3("directionalLight.position", glm::vec3(0.f, 1.f, 0.f));
        normalShader.setVec3("directionalLight.colour", glm::vec3(1.f, 1.f, 1.f));
        normalShader.setVec3("directionalLight.ambient", glm::vec3(1.f, 1.f, 1.f));
        normalShader.setVec3("directionalLight.specular", glm::vec3(.3f, .3f, .3f));
        normalShader.setVec3("directionalLight.diffuse", glm::vec3(1.f, 1.f, 1.f));
        backpackModel.draw(normalShader);

        geometryShader.use();
        geometryShader.setMat4("projection", projection);
        geometryShader.setMat4("view", view);
        geometryShader.setMat4("model", model);

        geometryShader.setUniformFloat("time", static_cast<float>(glfwGetTime()));
        backpackModel.draw(geometryShader);

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
