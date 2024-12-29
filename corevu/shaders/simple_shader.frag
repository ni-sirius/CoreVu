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
layout(set = 0, binding = 0) uniform GlobalUniformBufferObject // UBO
{
  mat4 projectionViewMatrix; // view * projection
  vec4 ambientLightColor;    // w as ambient intensity, xyz as color
  // vec3 directionToLight;     // for single direction light
  vec3 lightPosition;
  vec4 lightColor; // w as light intensity, xyz as color
}
ubo;

void main()
{
  vec3 directionToLight = ubo.lightPosition - fragPosWorld;
  float attenuation =
      1.0 / dot(directionToLight,
                directionToLight); // dot with itself is the distance squared

  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
  vec3 ambientLightColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 diffuseLight =
      lightColor *
      max(dot(normalize(
                  fragNormalWorld) /* necessary to normalize, because linear
                                      interpolation won't be normal inself */
              ,
              normalize(directionToLight)),
          0.0);

  // temporary comented out for direction lights
  // float intensity = ambientValue + max(dot(normalize(fragNormalWorld),
  // ubo.directionToLight),
  //                                     0.0); // temporary light intensity
  // fragColor = intensity * color;

  outColor = vec4(fragColor * (ambientLightColor + diffuseLight), 1.0);
}