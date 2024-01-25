#include <cstdlib>
#include <iostream>
#include <array>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const std::string vertex_shader_src{R"(#version 330 core
layout(location = 0) in vec3 attribute_position;

void
main()
{
  gl_Position = vec4(attribute_position.x, attribute_position.y, attribute_position.z, 1.0);
}
)"};

const std::string fragment_shader_src{R"(#version 330 core
out vec4 FragColor;

void
main()
{
  FragColor = vec4(1.0f, 0.5f, 0.5f, 1.f);
}
)"};

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

  unsigned int VBO;
  unsigned int VAO;
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  unsigned int shader_program;
  int success;
  char info_log[512];

  shader_program = glCreateProgram();
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  const char *vertex_shader_src_c = vertex_shader_src.c_str();
  const char *fragment_shader_src_c = fragment_shader_src.c_str();

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_src_c, nullptr);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if(!success)
    {
      glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
      std::cout << info_log << '\n';
      return EXIT_FAILURE;
    }
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_src_c, nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if(!success)
    {
      glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
      std::cout << info_log << '\n';
      return EXIT_FAILURE;
    }
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetShaderiv(shader_program, GL_LINK_STATUS, &success);
  if(!success)
    {
      glGetShaderInfoLog(shader_program, 512, nullptr, info_log);
      std::cout << info_log << '\n';
      return EXIT_FAILURE;
    }

  std::array<float, 9> vertices{ -0.5f, -0.5f, 0.0f,
                                  0.5f, -0.5f, 0.0f,
                                  0.0f,  0.5f, 0.0f };

  // Binding stuff to VAO.
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
  // 0 -> position vertex attribute
  // 3 -> size of the vertex atribute (vec3)
  // GL_FALSE -> we don't want (in this case) the data to be normalized.
  // 3 * sizeof(float) -> stride, i.e, space in bytes between each vertex attribute.
  // (void *)0 -> offset
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  while(!glfwWindowShouldClose(window))
    {
      process_input(window);

      // Render part.
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(shader_program);
      glBindVertexArray(VAO);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
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
