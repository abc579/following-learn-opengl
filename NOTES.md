# Core-profile and Immediate Mode

## Immediate-Mode (fixed function pipeline)

Old way of OpenGL programming. Less control. Inneficient.

## Core-profile Mode

New way. More flexible.

# State Machine

The state of OpenGL is called the context. We change the context manipulating buffers.

# IDs

We create objects and store their IDs to manipulate them later on.

# Frame

Each iteration inside the render loop is called a frame.

# Fragment

Data required for OpenGL to render a pixel.

# Graphics Pipeline

In OpenGL everything is in 3D space but the screen is in 2D, so OpenGL needs to transform 3D coords to 2D.

This process is managed by the graphics pipeline, which can be divided in two large parts:

1. Transforms 3D coords to 2D.
2. Transforms 2D coords into colored pixels.

More details about each part of the graphics pipeline:

1. **Vertex Shader**: Needs vertex data. Transforms 3D coordinates into 'different' 3D coordinates. Also allows us to do some attribute processing.
2. **Shape Assembly**: Takes as input all the vertices (or vertex) from the previous stage. Assembles and forms the primitive shape.
3. **Geometry Shader**: Can generate more triangles or shapes using the previous one as input.
4. **Rasterization**: Maps resulting primitives to the corresponding pixel on the final screen, resulting in fragments. Clipping is performed here.
5. **Fragment Shader**: Computes corresponding colors for each pixel. Good effects happen here.
6. **Tests and Blending**: Shadows, opacity, etc.

# Shaders

Small programs that the GPU runs are called shaders. The language is GLSL.

We can modify/inject shaders on these parts of the Graphics Pipeline:

- Vertex Shader.
- Geometry Shader.
- Fragment Shader.

# Vertex

Collection of data per 3D coordinate. This data is represented using vertex attributes that can contain any data.

# Primitives

Hints that we use to tell OpenGL what to draw: points, lines or triangles.

# Clipping

Discards all fragments that are outside our view. Improves performance.

# Required Shaders

In Modern OpenGL, we need to define at least one vertex and fragment shaders.

# Normalized coordinates

Coordinates that range between [-1.0, 1.0].

If coordinates are not normalized, i.e, they're not in that range, they won't get rendered.

# Normalized Device Coordinates (NDC)

Vertex Shader Coordinates after being processed -> Normalized Device Coordinates.

NDC coordinates are transformed to *screen-space coordinates* via the *viewport transform*.

# Vertex Buffer Objects (VBO)

To reserve and manipulate memory in the GPU, we use VBOs.

Sending data from the CPU to the GPU is slow. We want to minimize it, aka send large contents of data.

# Vertex Array Objects (VAO)

Helps us store vertex attribute pointers. Required.
