#version 450

layout(location = 0) in vec3 position;
//vec2 positions[3] = vec2[](vec2(0.0, -0.5),vec2(0.5, 0.5),vec2(-0.5, 0.5)); // hardcoded test
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
  mat4 transform;
  vec3 color;
} push;

void main()
{
  //gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0); // hardcoded test
  //gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0); // replaced with vec4 transform from push coordinates
  gl_Position = push.transform * vec4(position, 1.0); // note w=1.0 for position, 0.0 for direction
  fragColor = color;
}