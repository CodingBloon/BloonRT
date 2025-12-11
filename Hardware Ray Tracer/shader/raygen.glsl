#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_shader_image_load_formatted : enable

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 1, set = 0) uniform image2D image;
layout(binding = 2, set = 0) uniform CameraProperties {
    mat4 viewInverse;
    mat4 projInverse;
};

//Find way to input material information

#define MIN 0.0001
#define MAX 1000.0
layout(location = 0) rayPayloadEXT vec3 hitValue;

void main() {
    hitValue = vec3(0.0);
    imageStore(image, vec2(gl_LaunchIDEXT.xy), vec4(hitValue, 0.0));
}