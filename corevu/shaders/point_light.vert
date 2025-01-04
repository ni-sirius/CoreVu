#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, -1.0),
    vec2(-1.0, 1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 fragOffset;

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

void main()
{
  fragOffset = OFFSETS[gl_VertexIndex];

  // Solution in World space
  vec3 cameraRightWorld = {
      ubo.viewMatrix[0][0], ubo.viewMatrix[1][0], ubo.viewMatrix[2][0]};
  vec3 cameraUpWorld = {
      ubo.viewMatrix[0][1], ubo.viewMatrix[1][1], ubo.viewMatrix[2][1]};
  vec3 positionWorld = push.position.xyz +
                       cameraRightWorld * fragOffset.x * push.range +
                       cameraUpWorld * fragOffset.y * push.range;
  gl_Position =
      ubo.projectionMatrix * ubo.viewMatrix * vec4(positionWorld, 1.0);

  // Solution in View/camera space
  // vec4 lightInViewSpace = ubo.viewMatrix * vec4(ubo.lightPosition, 1.0);
  // vec4 positionViewSpace = lightInViewSpace + LIGHT_RADIUS * vec4(fragOffset,
  // 0.0, 0.0); gl_Position = ubo.projectionMatrix * positionViewSpace;
}