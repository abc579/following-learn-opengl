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
static void renderScene(const Shader&);
static void renderCube();

static constexpr unsigned int WindowWidth{ 1920 };
static constexpr unsigned int WindowHeight{ 1080 };
static bool shadows{ true };
static bool shadowsKeyPressed{ false };

static Camera camera(glm::vec3(0.f, 0.f, 3.f));
static float lastX{ static_cast<float>(WindowWidth) / 2.f };
static float lastY{ static_cast<float>(WindowHeight) / 2.f };
static bool firstMouse{ true };

static float deltaTime{ 0.f };
static float lastFrame{ 0.f };

// Put this as a global to change its position and draw the corresponding cube.
static glm::vec3 lightPos(0.f, 0.f, 0.f);

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
    glEnable(GL_CULL_FACE);

    Shader shader("./shaders/lightPointShadow.vs", "./shaders/lightPointShadow.fs");
    Shader simpleDepthShader("./shaders/pointShadow.vs", "./shaders/pointShadow.gs", "./shaders/pointShadow.fs");
    Shader pointLightCube("./shaders/pointLightCube.vs", "./shaders/pointLightCube.fs");

    auto woodTexture = loadTexture("./assets/wood.png");

    constexpr unsigned int ShadowWidth{ 1024 }, ShadowHeight{ 1024 };

    unsigned int depthMapFBO{ 0 };
    glGenFramebuffers(1, &depthMapFBO);

    unsigned int depthCubemap{ 0 };
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for(unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, ShadowWidth, ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader.use();
    shader.setUniformInt("diffuseTexture", 0);
    shader.setUniformInt("depthMap", 1);

    constexpr float NearPlane{ 1.0f };
    constexpr float FarPlane{ 25.0f };

    glm::mat4 shadowProj{ glm::perspective(glm::radians(90.f), static_cast<float>(ShadowWidth) / ShadowHeight, NearPlane, FarPlane) };

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        lightPos.z = static_cast<float>(sin(glfwGetTime() * .5f) * 3.f);

        glClearColor(.1f, .1f, .1f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const std::array<glm::mat4, 6> shadowTransforms{
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.f,  1.f,  0.f), glm::vec3(0.f,  0.f,  1.f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.f, -1.f,  0.f), glm::vec3(0.f,  0.f, -1.f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.f,  0.f,  1.f), glm::vec3(0.f, -1.f,  0.f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.f,  0.f, -1.f), glm::vec3(0.f, -1.f,  0.f)),
        };

        glViewport(0, 0, ShadowWidth, ShadowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        for(unsigned int i{ 0 }; i < shadowTransforms.size(); ++i) {
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        simpleDepthShader.setUniformFloat("far_plane", FarPlane);
        simpleDepthShader.setVec3("lightPos", lightPos);
        renderScene(simpleDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WindowWidth, WindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection{ glm::perspective(glm::radians(camera.zoom), (float)WindowWidth / (float)WindowHeight, .1f, 100.f) };
        glm::mat4 view{ camera.getViewMatrix() };
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.position);
        // Enable/disable shadows by pressing SPACE.
        shader.setUniformInt("shadows", shadows);
        shader.setUniformFloat("far_plane", FarPlane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader);
        // Render my light cube
        pointLightCube.use();
        pointLightCube.setMat4("projection", projection);
        pointLightCube.setMat4("view", view);
        glm::mat4 model{ glm::mat4(1.f) };
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(.3f));
        pointLightCube.setMat4("model", model);
        renderCube();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

void renderScene(const Shader& shader) {
    glm::mat4 model = glm::mat4(1.f);
    model = glm::scale(model, glm::vec3(5.f));
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    shader.setUniformInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    renderCube();
    shader.setUniformInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);

    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(4.f, -3.5f, 0.f));
    model = glm::scale(model, glm::vec3(.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(2.f, 3.f, 1.f));
    model = glm::scale(model, glm::vec3(.75f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(-3.f, -1.f, 0.f));
    model = glm::scale(model, glm::vec3(.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.f, 1.5f));
    model = glm::scale(model, glm::vec3(.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.f, -3.f));
    model = glm::rotate(model, glm::radians(60.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)));
    model = glm::scale(model, glm::vec3(.75f));
    shader.setMat4("model", model);
    renderCube();
}

void renderCube() {
    static unsigned int cubeVAO{ 0 };
    static unsigned int cubeVBO{ 0 };

    if(cubeVAO == 0) {
        std::array<float, 320> vertices{
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

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

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

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
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
    } else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed) {
        shadows = !shadows;
        shadowsKeyPressed = true;
    } else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        shadowsKeyPressed = false;
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
