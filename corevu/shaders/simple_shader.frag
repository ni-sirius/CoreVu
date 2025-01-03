#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push // Vulkan: limited to 128 bytes / two mat4
{
  mat4 modelMatrix;  // model
  mat4 normalMatrix; // transpose(inverse(mat3(model)))
}
push;

// Vulkan: uniform buffer has min 16KB(mobile divices) limit and max 64KB limit.
struct PointLight
{
  vec4 position; // ignore w
  vec4 color;    // w as light intensity
};
layout(set = 0, binding = 0) uniform GlobalUniformBufferObject // UBO
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
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
  vec3 diffuseLight /* here it's actually ambient light */ =
      ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 surfaceNormal =
      normalize(fragNormalWorld); /* necessary to normalize, because linear
                      interpolation won't be normal inself */

  for (int i = 0; i < ubo.pointLightCount; i++)
  {
    PointLight pointLight = ubo.pointLights[i];
    vec3 directionToLight = pointLight.position.xyz - fragPosWorld;
    float attenuation =
        1.0 / dot(directionToLight,
                  directionToLight); // dot with itself is the distance squared
    float cosAngleIncidence =
        max(dot(surfaceNormal, normalize(directionToLight)), 0.0);
    vec3 intensity /*aka lightColor */ =
        pointLight.color.xyz * pointLight.color.w * attenuation;

    diffuseLight += intensity * cosAngleIncidence;
  }

  outColor = vec4(fragColor * diffuseLight, 1.0);
}