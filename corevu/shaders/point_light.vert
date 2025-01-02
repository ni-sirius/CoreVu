#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject // UBO
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 ambientLightColor;    // w as ambient intensity, xyz as color
  // vec3 directionToLight;     // for single direction light
  vec3 lightPosition;
  vec4 lightColor; // w as light intensity, xyz as color
}
ubo;

const float LIGHT_RADIUS = 0.1;

void main()
{
  fragOffset = OFFSETS[gl_VertexIndex];

  // Solution in World space
  vec3 cameraRightWorld = {ubo.viewMatrix[0][0], ubo.viewMatrix[1][0], ubo.viewMatrix[2][0]};
  vec3 cameraUpWorld = {ubo.viewMatrix[0][1], ubo.viewMatrix[1][1], ubo.viewMatrix[2][1]};
  vec3 positionWorld = ubo.lightPosition + cameraRightWorld * fragOffset.x * LIGHT_RADIUS + cameraUpWorld * fragOffset.y * LIGHT_RADIUS;
  gl_Position = ubo.projectionMatrix * ubo.viewMatrix * vec4(positionWorld, 1.0);

  // Solution in View/camera space
  // vec4 lightInViewSpace = ubo.viewMatrix * vec4(ubo.lightPosition, 1.0);
  // vec4 positionViewSpace = lightInViewSpace + LIGHT_RADIUS * vec4(fragOffset, 0.0, 0.0);
  // gl_Position = ubo.projectionMatrix * positionViewSpace;
}