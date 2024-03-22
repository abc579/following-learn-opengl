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
static bool bloom { true };
static bool bloomKeyPressed{ false };
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

    // HERE
    Shader shader("./shaders/bloom.vs", "./shaders/bloom.fs");
    Shader shaderLight("./shaders/bloom.vs", "./shaders/lightBox.fs");
    Shader shaderBlur("./shaders/blur.vs", "./shaders/blur.fs");
    Shader shaderBloomFinal("./shaders/bloomFinal.vs", "./shaders/bloomFinal.fs");

    const auto woodTexture = loadTexture("./assets/wood.png", true);
    const auto containerTexture = loadTexture("./assets/container2.png", true);

    unsigned int hdrFBO{ 0 };
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    std::array<unsigned int, 2> colourBuffers;
    glGenTextures(2, &colourBuffers[0]);
    for(unsigned int i{ 0 }; i < colourBuffers.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, colourBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colourBuffers[i], 0);
    }

    // Create and attach depth buffer
    unsigned int rboDepth{ 0 };
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    std::array<unsigned int, 2> attachments{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(attachments.size(), &attachments[0]);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete, bro\n";
        return EXIT_FAILURE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Ping-pong framebuffer for blurring
    std::array<unsigned int, 2> pingPongFBO;
    std::array<unsigned int, 2> pingPongColourBuffers;
    glGenFramebuffers(2, &pingPongFBO[0]);
    glGenTextures(2, &pingPongColourBuffers[0]);
    for(unsigned int i{ 0 }; i < pingPongFBO.size(); ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingPongColourBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColourBuffers[i], 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer not complete, bro, n -> " << i << "\n";
            return EXIT_FAILURE;
        }
    }

    constexpr std::array<glm::vec3, 4> lightPositions{
        glm::vec3( 0.f, 0.5f,  1.5f),
        glm::vec3(-4.f, 0.5f, -3.f),
        glm::vec3( 3.f, 0.5f,  1.f),
        glm::vec3(-.8f, 2.4f, -1.f),
    };

    constexpr std::array<glm::vec3, 4> lightColours{
        glm::vec3( 5.f, 5.f,  5.f),
        glm::vec3(10.f, 0.f,  0.f),
        glm::vec3( 0.f, 0.f, 15.f),
        glm::vec3( 0.f, 5.f,  0.f),
    };

    // Setting textures for all shaders.
    shader.use();
    shader.setUniformInt("diffuseTexture", 0);
    shaderBlur.use();
    shaderBlur.setUniformInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setUniformInt("scene", 0);
    shaderBloomFinal.setUniformInt("bloomBlur", 1);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(.1f, .1f, .1f, 1.f);

        // 1. Render scene into floating point framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection{ glm::perspective(glm::radians(camera.zoom), static_cast<float>(WindowWidth) / WindowHeight, .1f, 100.f) };
        glm::mat4 view{ camera.getViewMatrix() };
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPosition", camera.position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        // Set lighting positions and colours
        for(unsigned int i{ 0 }; i < lightPositions.size(); ++i) {
            shader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            shader.setVec3("lights[" + std::to_string(i) + "].colour", lightColours[i]);
        }
        // Create large cube that acts as a floor
        glm::mat4 model{ glm::mat4(1.f) };
        model = glm::translate(model, glm::vec3(0.f, -1.f, 0.f));
        model = glm::scale(model, glm::vec3(12.5f, .5f, 12.5f));
        shader.setMat4("model", model);
        renderCube();
        // Rest of cubes
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(0.f, 1.5f, 0.f));
        model = glm::scale(model, glm::vec3(.5f));
        shader.setMat4("model", model);
        renderCube();

        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(2.f, 0.f, 1.f));
        model = glm::scale(model, glm::vec3(.5f));
        shader.setMat4("model", model);
        renderCube();

        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-1.f, -1.f, 2.f));
        model = glm::rotate(model, glm::radians(60.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)));
        shader.setMat4("model", model);
        renderCube();

        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-0.f, 2.7f, 4.f));
        model = glm::rotate(model, glm::radians(23.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)));
        model = glm::scale(model, glm::vec3(1.25f));
        shader.setMat4("model", model);
        renderCube();

        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-2.f, 1.f, -3.f));
        model = glm::rotate(model, glm::radians(124.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)));
        shader.setMat4("model", model);
        renderCube();

        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-3.f, 0.f, 0.f));
        model = glm::scale(model, glm::vec3(.5f));
        shader.setMat4("model", model);
        renderCube();

        // Show all light sources as bright cubes
        shaderLight.use();
        shaderLight.setMat4("projection", projection);
        shaderLight.setMat4("view", view);
        for(unsigned int i{ 0 }; i < lightPositions.size(); ++i) {
            model = glm::mat4(1.f);
            model = glm::translate(model, glm::vec3(lightPositions[i]));
            model = glm::scale(model, glm::vec3(0.25f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColour", lightColours[i]);
            renderCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Blur bright fragments with two-pass Gaussian blur
        bool horizontal{ true }, firstIteration{ true };
        constexpr unsigned int passes{ 10 };
        shaderBlur.use();
        for(unsigned int i{ 0 }; i < passes; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[horizontal]);
            shaderBlur.setUniformInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, firstIteration ? colourBuffers[1] : pingPongColourBuffers[!horizontal]);
            renderQuad();
            horizontal = !horizontal;
            if(firstIteration) {
                firstIteration = false;
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Now render floating point colour buffer to 2D quad and tonemap HDR colours to default's framebuffer LDR
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colourBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingPongColourBuffers[!horizontal]);
        shaderBloomFinal.setUniformBool("bloom", bloom);
        shaderBloomFinal.setUniformFloat("exposure", exposure);
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
    } else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed) {
        std::cout << "You're getting here\n";
        bloom = !bloom;
        bloomKeyPressed = true;
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
        bloomKeyPressed = false;
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
