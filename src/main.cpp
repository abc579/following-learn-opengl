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
[[nodiscard]] static unsigned int loadTexture(const std::string&, bool);

static constexpr unsigned int WindowWidth{ 1920 };
static constexpr unsigned int WindowHeight{ 1080 };

static Camera camera(glm::vec3(0.f, 0.f, 3.f));
static float lastX{ static_cast<float>(WindowWidth) / 2.f };
static float lastY{ static_cast<float>(WindowHeight) / 2.f };
static bool firstMouse{ true };
static void renderCube();
static void renderQuad();

static float deltaTime{ 0.f };
static float lastFrame{ 0.f };

static bool hdr{ true };
static bool hdrKeyPressed{ false };
static float exposure{ 1.f };

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

    Shader shader("./shaders/hdrLighting.vs", "./shaders/hdrLighting.fs");
    Shader hdrShader("./shaders/hdrLighting2.vs", "./shaders/hdrLighting2.fs");

    const auto woodTexture = loadTexture("./assets/wood.png", true);

    unsigned int hdrFBO{ 0 };
    glGenFramebuffers(1, &hdrFBO);

    // Floating point colour buffer.
    unsigned int colourBuffer{ 0 };
    glGenTextures(1, &colourBuffer);
    glBindTexture(GL_TEXTURE_2D, colourBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Depth buffer (renderbuffer)
    unsigned int rboDepth{ 0 };
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);
    // Attach buffers to renderbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colourBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer isn't complete, bro.\n";
        return EXIT_FAILURE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Lighting data
    constexpr std::array<glm::vec3, 4> lightPositions{
        glm::vec3( 0.f,   0.f, 49.5f), // back light
        glm::vec3(-1.4f, -1.9f, 9.f),
        glm::vec3( 0.f,  -1.8f, 4.f),
        glm::vec3( 0.8f, -1.7f, 6.f),
    };

    constexpr std::array<glm::vec3, 4> lightColours{
        glm::vec3(200.f, 200.f, 200.f),
        glm::vec3(  .1f,   0.f, 0.f),
        glm::vec3(  0.f,   0.f, .2f),
        glm::vec3(  0.f,   .1f, 0.f),
    };

    shader.use();
    shader.setUniformInt("diffuseTexture", 0);

    hdrShader.use();
    hdrShader.setUniformInt("hdrBuffer", 0);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(.1f, .1f, .1f, 1.f);

        // First pass, render stuff onto framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection{ glm::perspective(glm::radians(camera.zoom), static_cast<float>(WindowWidth) / WindowHeight, .1f, 100.f) };
        glm::mat4 view{ camera.getViewMatrix() };
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        for(unsigned int i{ 0 }; i < lightPositions.size(); ++i) {
            shader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            shader.setVec3("lights[" + std::to_string(i) + "].colour", lightColours[i]);
        }
        shader.setVec3("viewPosition", camera.position);
        // Tunnel.
        glm::mat4 model{ glm::mat4(1.f) };
        model = glm::translate(model, glm::vec3(0.f, 0.f, 25.f));
        model = glm::scale(model, glm::vec3(2.5f, 2.5f, 27.5f));
        shader.setMat4("model", model);
        shader.setUniformInt("inverseNormals", true);
        renderCube();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Second pass, render colour buffer to 2D quad and tonemap HDR colours to default framebuffer's clamped colour range (LDR).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colourBuffer);
        hdrShader.setUniformInt("hdr", hdr);
        hdrShader.setUniformFloat("exposure", exposure);
        renderQuad();

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
    } else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed) {
        hdr = !hdr;
        hdrKeyPressed = true;
    } else if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if(exposure > 0.f) {
            exposure -= .001f;
        } else {
            exposure = 0.f;
        }
    } else if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        exposure += .001f;
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        hdrKeyPressed = false;
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

unsigned int loadTexture(const std::string& path, const bool gammaCorrection) {
    unsigned int textureID{ 0 };
    glGenTextures(1, &textureID);

    int width{ 0 }, height{ 0 }, nrComponents{ 0 };
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        std::cerr << "Texture failed to load at path: " << path << '\n';
        stbi_image_free(data);
    }

    GLenum format{ GL_RED };
    GLenum internalFormat{ GL_RED };

    if(nrComponents == 1) {
        internalFormat = format = GL_RED;
    } else if(nrComponents == 3) {
        internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
        format = GL_RGB;
    } else if(nrComponents == 4) {
        internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
        format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

static void renderCube() {
    static unsigned int cubeVAO{ 0 }, cubeVBO{ 0 };
    if(cubeVAO == 0) {
        float cubeVertices[] = {
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
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

static void renderQuad() {
    static unsigned int quadVAO{ 0 }, quadVBO{ 0 };
    if(quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
