#version 330 core
layout(location = 0) in vec3 attribute_position;

void
main()
{
  gl_Position = vec4(attribute_position.x, attribute_position.y, attribute_position.z, 1.0);
}
