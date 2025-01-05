#version 450

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push // Vulkan: limited to 128 bytes / two mat4
{
  vec4 color;
  vec4 position;
  float range; /* aka radius */
}
push;

struct PointLight
{
  vec4 position; // ignore w
  vec4 color;    // w as light intensity
};
layout(set = 0, binding = 0) uniform GlobalUniformBufferObject // UBO
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 invViewMatrix;
  vec4 ambientLightColor; // w as ambient intensity, xyz as color
  // vec3 directionToLight;     // for single direction light
  PointLight pointLights[10]; // instead of hardcoding light amount we can use
                              // Vulkan's Specialization Constants to pass
                              // values at time of pipeline creation
  int pointLightCount;
}
ubo;

const float PI = 3.14159265359;

void main()
{
  // shaping to be a cicrle
  float distance = sqrt(dot(fragOffset, fragOffset));
  if (distance >= 1.0)
    discard;

  float cosTerm = (cos(distance * PI) + 1.0) *
                0.5; // convinent to use cos here. distance -> 1 at edge,
                     // cos(distance * PI) -> -1, <cos res> +1 and * 0.5 -> 0.
  outColor = vec4(push.color.xyz + cosTerm, cosTerm);
}