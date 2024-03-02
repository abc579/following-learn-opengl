#include <cstdlib>
#include <cmath>
#include <iostream>
#include <array>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Shader.hpp>
#include <Camera.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow*, int, int);
void process_input(GLFWwindow*);
void mouse_callback(GLFWwindow*, double, double);
void scroll_callback(GLFWwindow* , double, double);

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

    stbi_set_flip_vertically_on_load(true);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto *window = glfwCreateWindow(windowWidth, windowHeight, "Learn OpenGL", nullptr, nullptr);
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

    Shader lightingShader("./shaders/lightingShader.vs", "./shaders/lightingShader.fs");
    Shader lightSourceShader("./shaders/lightingSourceShader.vs", "./shaders/lightSourceShader.fs");

    unsigned int VBO = 0;
    // 0 -> light source, 1 -> cube with colour
    std::array<unsigned int, 2> VAO{ 0 };
    unsigned int EBO = 0;

    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(2, &VAO[0]);

    glGenBuffers(1, &VBO);

    glGenBuffers(1, &EBO);

    std::array<float, 240> vertices{
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindVertexArray(VAO[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    std::array<glm::vec3, 1> cubePositions{
        glm::vec3(1.2f,  1.f,  2.0f)  // lightPosition
    };

    glm::vec3 lightColour(1.f, 1.f, 1.f);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Object
        lightingShader.use();
        lightingShader.setVec3("objectColour", 1.f, 0.1f, .1f);
        lightingShader.setVec3("lightColour", lightColour);
        lightingShader.setVec3("lightPosition", cubePositions[0]);
        lightingShader.setVec3("viewerPosition", camera.position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float) windowWidth / (float) windowHeight, 0.1f, 100.f);
        glm::mat4 view = camera.getViewMatrix();

        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Light Source
        lightSourceShader.use();
        lightSourceShader.setVec3("colour", lightColour);
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        model = glm::mat4(1.f);
        cubePositions[0].x = std::sin(static_cast<float>(glfwGetTime()));
        cubePositions[0].z = std::cos(static_cast<float>(glfwGetTime()));
        model = glm::translate(model, cubePositions[0]);
        model = glm::scale(model, glm::vec3(.2f));
        lightSourceShader.setMat4("model", model);

        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 -> number of vertices in a cube

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
