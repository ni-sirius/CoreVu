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
  // shared precalculations
  vec3 diffuseLight /* here it's actually ambient light */ =
      ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal =
      normalize(fragNormalWorld); /* necessary to normalize, because linear
                      interpolation won't be normal inself */

  vec3 cameraWorldPos = ubo.invViewMatrix[3].xyz;
  vec3 viewDir = normalize(cameraWorldPos - fragPosWorld);

  for (int i = 0; i < ubo.pointLightCount; i++)
  {
    // individual precalculations
    PointLight pointLight = ubo.pointLights[i];
    vec3 directionToLight = pointLight.position.xyz - fragPosWorld;
    float attenuation =
        1.0 / dot(directionToLight,
                  directionToLight); // dot with itself is the distance squared
    vec3 normDirectionToLight = normalize(directionToLight);

    // diffuse light
    float cosAngleIncidence =
        max(dot(surfaceNormal, normDirectionToLight), 0.0);
    vec3 intensity /*aka lightColor */ =
        pointLight.color.xyz * pointLight.color.w * attenuation;

    diffuseLight += intensity * cosAngleIncidence;

    // specular light
    vec3 halfVector = normalize(normDirectionToLight + viewDir);
    float blinnTerm = dot(surfaceNormal, halfVector);
    blinnTerm = clamp(blinnTerm, 0.0, 1.0); // prevent negative values on opposite side
    blinnTerm = pow(blinnTerm, 512.0); // shininess - the higher the numer the sharper the highlight
    specularLight += intensity * blinnTerm;
  }

  // final color C = MaterialDiffuseColor * DiffuseLight + MaterialSpecularColor * SpecularLight
  vec3 specularControlTerm = fragColor; //TODO pass to shader, but it's fine now for metallic materials
  outColor = vec4(fragColor * diffuseLight + specularControlTerm * specularLight, 1.0);
}