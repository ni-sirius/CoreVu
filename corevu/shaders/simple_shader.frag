#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push // Vulkan: limited to 128 bytes / two mat4
{
  mat4 modelMatrix;  // model
  mat4 normalMatrix; // transpose(inverse(mat3(model)))
}
push;

// Vulkan: uniform buffer has min 16KB limit and max 64KB limit.

void main()
{
  outColor = vec4(fragColor, 1.0);
}