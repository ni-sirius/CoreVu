#version 450

//layout (location = 0) in vec3 fragColor; // commented to test push constants
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
  vec2 offset;
  vec3 color;
} push;

void main()
{
  outColor = vec4(/* fragColor */push.color, 1.0); // commented to test push constants
}