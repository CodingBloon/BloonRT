#pragma once

#include "shaderio.slang"
#include "brdf.slang"
#include "utils/mesh.slang"
#include "utils/constants.slang"
#include "utils/random.slang"
#include "utils/light.slang"

RaytracingAccelerationStructure topLevelAS;
RWTexture2D<float4> outImage;
ConstantBuffer<UniformBuffer> uniformBuffer;
GLSLShaderStorageBuffer<SceneBuffer> sceneBuffer;

struct HitPayload {
    float3 color;
    float weight;
    int depth;
    uint seed;

    float3 rayOrigin;
    float3 rayDirection;
};

struct ShadowPayload {
    int depth;
};

float testShadow(float3 worldPos, float3 worldNormal, float3 lightDirection) {
    RayDesc shadowRay;
    shadowRay.Origin = worldPos + worldNormal * 0.001;
    shadowRay.Direction = normalize(lightDirection);
    shadowRay.TMin = 0.001;
    shadowRay.TMax = length(lightDirection);

    ShadowPayload shadowPayload;
    shadowPayload.depth = 0;

    // uses shadow miss shader to reduce the payload and computational cost
    TraceRay(topLevelAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xff, 0, 0, 2, shadowRay, shadowPayload);

    return shadowPayload.depth != MISS_DEPTH ? 0.0 : 1.0;
}

float3 shadePoint(Mesh::Material *material, float3 normal, float3 view, float3 worldPos, uint seed) {
    uint numLights = (uint)sceneBuffer.numLights;

    // the algorithm currently selects one light at random
    // this will later be replaced with Light BVHs in order to boost performance and reduce noise
    uint index = rand(seed, numLights - 1);
    Light::Light light = Light::processLight(sceneBuffer.lightBuffer, sceneBuffer.lightByteStride, index, worldPos);
    float3 L = normalize(light.direction);
    return BRDF::BRDF(material, normal, view, L) * light.color * light.intensity * testShadow(worldPos, normal, light.direction);
}

[shader("raygeneration")]
void rgenMain() {
    float2 launchID = (float2)DispatchRaysIndex().xy;
    float2 launchSize = (float2)DispatchRaysDimensions().xy;
    const uint rayFlags = 0;
    uint seed = hash(uint3(uint2(launchID.xy), uniformBuffer.frameIndex));

    const float2 clipCoords = launchID / launchSize * 2.0f - 1.0f;
    const float4 viewCoords = mul(float4(clipCoords, 1.0f), uniformBuffer.projInverse);

    RayDesc ray;
    ray.Origin = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), uniformBuffer.viewInverse).xyz;
    ray.Direction = mul(float4(normalize(viewCoords.xyz), 0.0f), uniformBuffer.viewInverse).xyz;
    ray.TMin = 0.001f;
    ray.TMax = INFINITE;

    HitPayload payload;
    payload.color = float3(0.0f);
    payload.weight = 1.0f;
    payload.seed = seed;
    payload.depth = 0;

    float3 accumulated = float3(0.0f);
    while (payload.depth < uniformBuffer.depthMax && payload.weight > ZERO_WEIGHT) {
        float prevWeight = payload.weight;
        TraceRay(topLevelAS, rayFlags, 0xff, 0, 0, 0, ray, payload);
        accumulated += payload.color;
        ray.Direction = payload.rayDirection;
        ray.Origin = payload.rayOrigin;
    }

    outImage[int2(launchID)] = float4(accumulated, 1.0f);
}

[shader("closesthit")]
void rchitMain(inout HitPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    uint instanceID = InstanceIndex();
    uint meshID = InstanceID();
    uint triID = PrimitiveIndex();

    Mesh::Triangle tri = Mesh::getTriangeInformation(sceneBuffer.instanceBuffer, sceneBuffer.instanceByteStride, sceneBuffer.vertexByteStride, meshID, triID, barycentrics);        
    Mesh::Material material = Mesh::getMaterial(sceneBuffer.materialBuffer, sceneBuffer.materialByteStride, sceneBuffer.instanceBuffer, sceneBuffer.instanceByteStride, instanceID);

    float3 worldPos = float3(mul(float4(tri.pos, 1.0), ObjectToWorld4x3()));
    float3 worldNormal = normalize(mul(WorldToObject4x3(), tri.normal).xyz);
    float3 N = normalize(worldNormal);
    float3 V = WorldRayDirection();

    if (dot(N, -V) < 0.0)
        N = -N;

    float pdf;
    payload.color = shadePoint(&material, N, -V, worldPos, payload.seed);
    payload.rayOrigin = worldPos + N * 0.001f;
    payload.rayDirection = Sampling::sample_surface(&material, N, V, payload.seed, pdf);
    // payload.weight = material.metallic * pdf;
    payload.weight = pdf;
    payload.depth++;
    //payload.color = shadePoint(&material, N, V, worldPos, payload.seed);
    /*payload.color = shadePoint(&material, N, V, worldPos, payload.seed);
    payload.rayOrigin = worldPos + N * 0.001f;
    payload.rayDirection = Sampling::sample_surface(&material, N, V, payload.seed, pdf);
    payload.weight = BRDF::reflectivity(&material) * pdf;*/
}

[shader("miss")]
void rmissMain(inout HitPayload payload) {
    payload.color = float3(0.0f);
    payload.depth = MISS_DEPTH;
}

[shader("miss")]
void rmissShadowMain(inout ShadowPayload payload) {
    payload.depth = MISS_DEPTH;
}