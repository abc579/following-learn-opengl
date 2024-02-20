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

const std::string fragment_shader_src_second_triangle{R"(#version 330 core
out vec4 FragColor;

void
main()
{
  FragColor = vec4(1.0f, 1.0f, 0.0f, 1.f);
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

  unsigned int VBO, VBO2;
  unsigned int VAO, VAO2;
  unsigned int EBO, EBO2;
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  unsigned int fragment_shader_second_triangle;
  unsigned int shader_program;
  unsigned int shader_program_second_triangle;
  int success;
  char info_log[512];

  shader_program = glCreateProgram();
  shader_program_second_triangle = glCreateProgram();

  glGenVertexArrays(1, &VAO);
  glGenVertexArrays(1, &VAO2);

  glGenBuffers(1, &VBO);
  glGenBuffers(1, &VBO2);

  glGenBuffers(1, &EBO);
  glGenBuffers(1, &EBO2);

  const char *vertex_shader_src_c = vertex_shader_src.c_str();
  const char *fragment_shader_src_c = fragment_shader_src.c_str();
  const char *fragment_shader_src_second_triangle_c = fragment_shader_src_second_triangle.c_str();

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

  fragment_shader_second_triangle = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_second_triangle, 1, &fragment_shader_src_second_triangle_c, nullptr);
  glCompileShader(fragment_shader_second_triangle);
  glGetShaderiv(fragment_shader_second_triangle, GL_COMPILE_STATUS, &success);
  if(!success)
    {
      glGetShaderInfoLog(fragment_shader_second_triangle, 512, nullptr, info_log);
      std::cout << info_log << '\n';
      return EXIT_FAILURE;
    }
  glAttachShader(shader_program_second_triangle, vertex_shader);
  glAttachShader(shader_program_second_triangle, fragment_shader_second_triangle);
  glLinkProgram(shader_program_second_triangle);
  glGetShaderiv(shader_program_second_triangle, GL_LINK_STATUS, &success);
  if(!success)
    {
      glGetShaderInfoLog(shader_program_second_triangle, 512, nullptr, info_log);
      std::cout << info_log << '\n';
      return EXIT_FAILURE;
    }

  std::array<float, 15> vertices{  -0.5f,  0.0f, 0.0f,
                                   0.0f,  0.0f, 0.0f,
                                   -0.25f, 0.5f, 0.0f,
                                   0.5f,  0.0f, 0.0f,
                                   0.25f, 0.5f, 0.0f };

  std::array<unsigned int, 3> indicesFirstTriangle{ 0, 1, 2 };

  std::array<unsigned int, 3> indicesSecondTriangle{ 1, 3, 4 };

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesFirstTriangle.size(), &indicesFirstTriangle[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glBindVertexArray(VAO2);
  glBindBuffer(GL_ARRAY_BUFFER, VBO2);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesSecondTriangle.size(), &indicesSecondTriangle[0], GL_STATIC_DRAW);
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

      glBindVertexArray(VAO);
      glUseProgram(shader_program);
      glDrawElements(GL_TRIANGLES, indicesFirstTriangle.size(), GL_UNSIGNED_INT, 0);

      glBindVertexArray(VAO2);
      glUseProgram(shader_program_second_triangle);
      glDrawElements(GL_TRIANGLES, indicesSecondTriangle.size(), GL_UNSIGNED_INT, 0);

      glBindVertexArray(0);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glDeleteShader(fragment_shader_second_triangle);
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
