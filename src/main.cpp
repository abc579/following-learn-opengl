#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.hpp>
#include <Camera.hpp>
#include <Model.hpp>

#include <iostream>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static void framebuffer_size_callback(GLFWwindow*, int, int);
static void mouse_callback(GLFWwindow*, double, double);
static void scroll_callback(GLFWwindow*, double, double);
static void processInput(GLFWwindow*);
[[nodiscard]] static unsigned int loadTexture(const std::string&);

static constexpr unsigned int WindowWidth{ 1920 };
static constexpr unsigned int WindowHeight{ 1080 };

static Camera camera(glm::vec3(0.f, 0.f, 3.f));
static float lastX{ static_cast<float>(WindowWidth) / 2.f };
static float lastY{ static_cast<float>(WindowHeight) / 2.f };
static bool firstMouse{ true };

static float deltaTime{ 0.f };
static float lastFrame{ 0.f };

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto* window = glfwCreateWindow(WindowWidth, WindowHeight, "Unloved", nullptr, nullptr);
    if(!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);

    // Positions.
    glm::vec3 upperLeft(-1.f, 1.f, 0.f); // pos1
    glm::vec3 bottomLeft(-1.f, -1.f, 0.f); // pos2
    glm::vec3 bottomRight(1.f, -1.f, 0.f); // pos3
    glm::vec3 upperRight(1.f, 1.f, 0.f); // pos4

    // Texture coordinates.
    glm::vec2 uv1(0.f, 1.f);
    glm::vec2 uv2(0.f, 0.f);
    glm::vec2 uv3(1.f, 0.f);
    glm::vec2 uv4(1.f, 1.f);

    // Normal vector.
    glm::vec3 normal(0.f, 0.f, 1.f);

    glm::vec3 edge1{ bottomLeft - upperLeft };
    glm::vec3 edge2{ bottomRight - upperLeft };
    glm::vec2 deltaUV1{ uv2 - uv1 };
    glm::vec2 deltaUV2{ uv3 - uv1 };

    float f{ 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y) };

    // Tangent and bitangent for the first triangle.
    const glm::vec3 tangent1{
        glm::vec3(f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                  f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                  f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z))
    };

    const glm::vec3 bitangent1{
        glm::vec3(f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
                  f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
                  f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z))
    };

    edge1 = bottomRight - upperLeft;
    edge2 = upperRight - upperLeft;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    // Tangent and bitangent for the second triangle.
    const glm::vec3 tangent2{
        glm::vec3(f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                  f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                  f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z))
    };

    const glm::vec3 bitangent2{
        glm::vec3(f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
                  f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
                  f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z))
    };

    const std::array<float, 84> planeVertices{
        // positions                                 // normal                     // texcoords  // tangent                          // bitangent
        upperLeft.x, upperLeft.y, upperLeft.z,       normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        bottomLeft.x, bottomLeft.y, bottomLeft.z,    normal.x, normal.y, normal.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        bottomRight.x, bottomRight.y, bottomRight.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

        upperLeft.x, upperLeft.y, upperLeft.z,       normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        bottomRight.x, bottomRight.y, bottomRight.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        upperRight.x, upperRight.y, upperRight.z,    normal.x, normal.y, normal.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
    };

    constexpr std::array<float, 108> cubeVertices{
        // back face
        -1.0f, -1.0f, -1.0f, // bottom-left
         1.0f,  1.0f, -1.0f, // top-right
         1.0f, -1.0f, -1.0f, // bottom-right
         1.0f,  1.0f, -1.0f, // top-right
        -1.0f, -1.0f, -1.0f, // bottom-left
        -1.0f,  1.0f, -1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f, // bottom-left
         1.0f, -1.0f,  1.0f, // bottom-right
         1.0f,  1.0f,  1.0f, // top-right
         1.0f,  1.0f,  1.0f, // top-right
        -1.0f,  1.0f,  1.0f, // top-left
        -1.0f, -1.0f,  1.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, // top-right
        -1.0f,  1.0f, -1.0f, // top-left
        -1.0f, -1.0f, -1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f, // top-left
         1.0f, -1.0f, -1.0f, // bottom-right
         1.0f,  1.0f, -1.0f, // top-right
         1.0f, -1.0f, -1.0f, // bottom-right
         1.0f,  1.0f,  1.0f, // top-left
         1.0f, -1.0f,  1.0f, // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f, // top-right
         1.0f, -1.0f, -1.0f, // top-left
         1.0f, -1.0f,  1.0f, // bottom-left
         1.0f, -1.0f,  1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, // bottom-right
        -1.0f, -1.0f, -1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f, // top-left
         1.0f,  1.0f , 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f, // top-right
         1.0f,  1.0f,  1.0f, // bottom-right
        -1.0f,  1.0f, -1.0f, // top-left
        -1.0f,  1.0f,  1.0f, // bottom-left
    };

    unsigned int planeVAO{ 0 }, planeVBO{ 0 };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int cubeVAO{ 0 }, cubeVBO{ 0 };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Shader shader("./shaders/brickwall.vs", "./shaders/brickwall.fs");
    Shader lightCubeShader("./shaders/pointLightCube.vs", "./shaders/pointLightCube.fs");

    const auto brickwallTexture{ loadTexture("./assets/bricks2.jpg") };
    const auto brickwallNormalTexture{ loadTexture("./assets/bricks2_normal.jpg") };
    const auto brickwallDisplacementTexture{ loadTexture("./assets/bricks2_disp.jpg") };

    shader.use();
    shader.setUniformInt("diffuseTexture", 0);
    shader.setUniformInt("normalTexture", 1);
    shader.setUniformInt("depthTexture", 2);
    shader.setUniformFloat("heightScale", .1f);

    glm::vec3 lightPosition{ glm::vec3(.5f, 1.f, .3f) };

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(.3f, .3f, .3f, 1.f);

        glm::mat4 projection{ glm::perspective(glm::radians(camera.zoom), static_cast<float>(WindowWidth) / WindowHeight, .1f, 100.f) };
        glm::mat4 view{ camera.getViewMatrix() };
        glm::mat4 model{ glm::mat4(1.f) };

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);
        shader.setVec3("lightPosition", lightPosition);
        shader.setVec3("viewerPosition", camera.position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickwallTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickwallNormalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brickwallDisplacementTexture);

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        lightCubeShader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        model = glm::translate(model, lightPosition);
        model = glm::scale(model, glm::vec3(.1f));
        shader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::FORWARD, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::BACKWARD, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::LEFT, deltaTime);
    } else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(CameraMovementOptions::RIGHT, deltaTime);
    }
}

void framebuffer_size_callback([[maybe_unused]]GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback([[maybe_unused]]GLFWwindow* window, double xposIn, double yposIn) {
    float xpos{ static_cast<float>(xposIn) };
    float ypos{ static_cast<float>(yposIn) };

    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset{ xpos - lastX };
    float yoffset{ lastY - ypos }; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback([[maybe_unused]]GLFWwindow* window, [[maybe_unused]]double xoffset, double yoffset) {
    camera.processMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const std::string& path) {
    unsigned int textureID{ 0 };
    glGenTextures(1, &textureID);

    int width{ 0 }, height{ 0 }, nrComponents{ 0 };
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        std::cerr << "Texture failed to load at path: " << path << '\n';
        stbi_image_free(data);
    }

    GLenum format{ GL_RED };

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

    // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}
