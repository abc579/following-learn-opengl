#include <cstdlib>
#include <cmath>
#include <iostream>
#include <array>
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

    Shader modelLoadingShader("./shaders/modelLoading.vs", "./shaders/modelLoading.fs");
    Model ourModel("./assets/backpack/backpack.obj");

    glEnable(GL_DEPTH_TEST);

    constexpr glm::vec3 sunPosition(0.f, 10.f, 0.f);
    constexpr glm::vec3 lightColour(1.f, 1.f, 1.f);
    constexpr glm::vec3 lightColour2(0.f, 0.f, 1.f);

    constexpr glm::vec3 directionalLightAmbientValues(.1f, .1f, .1f);
    constexpr glm::vec3 directionalLightDiffuseValues(.5f, .5f, .5f);
    constexpr glm::vec3 directionalLightSpecularValues(1.f, 1.f, 1.f);

    // constexpr glm::vec3 pointLightAmbientValues(.1f, .1f, .1f);
    // constexpr glm::vec3 pointLightDiffuseValues(.4f, .4f, .4f);
    // constexpr glm::vec3 pointLightSpecularValues(1.f, 1.f, 1.f);

    constexpr glm::vec3 spotlightLightAmbientValues(.1f, .1f, .1f);
    constexpr glm::vec3 spotlightLightDiffuseValues(.5f, .5f, .5f);
    constexpr glm::vec3 spotlightLightSpecularValues(1.f, 1.f, 1.f);

    constexpr float lightCutOffAngle{ glm::cos(glm::radians(12.f)) };
    constexpr float lightOuterCutOffAngle{ glm::cos(glm::radians(17.5f)) };
    constexpr float attenuationConstantFactor{ 1.f };
    constexpr float attenuationLinearFactor{ .027f };
    constexpr float attenuationQuadraticFactor{ .0028f };

    while(!glfwWindowShouldClose(window)) {
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        glClearColor(.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelLoadingShader.use();
        modelLoadingShader.setVec3("directionalLight.position", sunPosition);
        modelLoadingShader.setVec3("directionalLight.colour", lightColour);
        modelLoadingShader.setVec3("directionalLight.ambient", directionalLightAmbientValues);
        modelLoadingShader.setVec3("directionalLight.specular", directionalLightSpecularValues);
        modelLoadingShader.setVec3("directionalLight.diffuse", directionalLightDiffuseValues);

        modelLoadingShader.setVec3("spotlightLight.position", camera.position);
        modelLoadingShader.setVec3("spotlightLight.direction", camera.front);
        modelLoadingShader.setVec3("spotlightLight.colour", lightColour2);
        modelLoadingShader.setVec3("spotlightLight.ambient", spotlightLightAmbientValues);
        modelLoadingShader.setVec3("spotlightLight.specular", spotlightLightSpecularValues);
        modelLoadingShader.setVec3("spotlightLight.diffuse", spotlightLightDiffuseValues);
        modelLoadingShader.setUniformFloat("spotlightLight.constant", attenuationConstantFactor);
        modelLoadingShader.setUniformFloat("spotlightLight.linear", attenuationLinearFactor);
        modelLoadingShader.setUniformFloat("spotlightLight.quadratic", attenuationQuadraticFactor);
        modelLoadingShader.setUniformFloat("spotlightLight.cutOff", lightCutOffAngle);
        modelLoadingShader.setUniformFloat("spotlightLight.outerCutOff", lightOuterCutOffAngle);

        // render the loaded model
        const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 100.f);
        const glm::mat4 view = camera.getViewMatrix();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down

        modelLoadingShader.setMat4("projection", projection);
        modelLoadingShader.setMat4("view", view);
        modelLoadingShader.setMat4("model", model);

        ourModel.draw(modelLoadingShader);

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
