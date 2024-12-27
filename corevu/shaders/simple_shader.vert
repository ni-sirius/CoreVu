#version 450

// vec2 positions[3] = vec2[](vec2(0.0, -0.5),vec2(0.5, 0.5),vec2(-0.5, 0.5));
// // hardcoded test
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;

layout(location = 0) out vec3 fragColor;

// Input uniform data
/** NOTE on vulkan speeds for 5000 blocks to render:
Using push-constants 2.11ms
Using 1 VkBuffer and keeping it mapped all the time 2.22ms
Using 1 VkBuffer and mapping/unmapping per frame 2.25ms
Using multiple VkBuffers, and keeping them all mapped 2.26ms
Using multiple VkBuffers, and mapping/unmapping every frame 2.83ms
*/
layout(push_constant) uniform Push
{
  mat4 transform;    // projection * view * model
  mat4 normalMatrix; // transpose(inverse(mat3(model)))
}
push;

// Vulkan: uniform buffer has min 16KB(mobile divices) limit and max 64KB limit.

// Code and constants
const vec3 lightDir =
    normalize(vec3(1.0, -3.0, -1.0)); // hardcoded light direction test
const float ambientValue = 0.1;       // hardcoded ambient value test

void main()
{
  // gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0); // hardcoded test
  // gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0); //
  // replaced with vec4 transform from push coordinates

  gl_Position =
      push.transform *
      vec4(position, 1.0); // note w=1.0 for position, 0.0 for direction

  // Normals scaling handling
  // *1 this option is valid only if all object could be scaled uniformly e.g.
  // use float scale instead of vec3 scale. vec3 normalsWorld =
  // normalize(mat3(push.modelTransform) * normal);
  // *2 this option is valid, but
  // it's not recommented because of gpu overhead
  // mat3 normalMatrix = transpose(inverse(mat3(push.modelTransform)));
  // vec3 normalsWorld = normalize(normalMatrix * normal);
  // *3 this option is valid and recommended, but adds addition engine
  // complexity) NOTE: The optimal way would be using approach 1 if object is
  // static an won't be scaled non-uniformly. If object is scaled on creation
  // and not scaled - use approach 3. If object is scaled in runtime - use
  // approach 2.
  vec3 normalsWorld = normalize(mat3(push.normalMatrix) * normal);

  float intensity = ambientValue + max(dot(normalsWorld, lightDir),
                                       0.0); // temporary light intensity

  fragColor = intensity * color;
}