#include <cstdlib>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow *, int, int);

int
main()
{
  std::ios::sync_with_stdio(false);

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto *window = glfwCreateWindow(800, 600, "Learn OpenGL", nullptr, nullptr);
  if(!window)
    {
      std::cout << "Couldn't create window!\n";
      glfwTerminate();
      return EXIT_FAILURE;
    }

  glfwMakeContextCurrent(window);

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD.\n";
      return EXIT_FAILURE;
    }

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glViewport(0, 0, 800, 600);

  while(!glfwWindowShouldClose(window))
    {
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  glfwTerminate();

  return EXIT_SUCCESS;
}

void
framebuffer_size_callback([[maybe_unused]]GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}
