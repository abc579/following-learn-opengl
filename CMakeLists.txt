cmake_minimum_required(VERSION 3.15)

project(following-learn-opengl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(following-learn-opengl src/glad.c src/main.cpp)

target_include_directories(following-learn-opengl PRIVATE ${CMAKE_SOURCE_DIR}/include)

# -lglfw3 -ldl -lm -lGL -lGLU -lX11
target_link_libraries(following-learn-opengl PRIVATE
  ${CMAKE_SOURCE_DIR}/deps/libglfw3.a
  Threads::Threads
  ${OPENGL_LIBRARIES}
  ${X11_LIBRARIES}
)
