#include <cstdlib>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow *, int, int);
void process_input(GLFWwindow *);

int
main()
{
  std::ios::sync_with_stdio(false);

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto *window = glfwCreateWindow(1600, 1200, "Learn OpenGL", nullptr, nullptr);
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

  glViewport(0, 0, 1600, 1200);

  while(!glfwWindowShouldClose(window))
    {
      process_input(window);

      glClearColor(1.f, 1.f, 1.f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

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

void
process_input(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
      glfwSetWindowShouldClose(window, true);
    }
}
