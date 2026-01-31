#pragma once

#include "../shaderio.slang"

namespace Mesh {
    struct Material {
        float3 color;
        float subsurface;
        float metallic;
        float roughness;
        float specular;
        float specularTint;
        float anisotropic;
        float sheen;
        float sheenTint;
        float clearCoat;
        float clearCoatGloss;
    };
    
    struct Triangle {
        float3 pos;
        float3 normal;
        float2 uv;
    };
    
    __generic<T : IFloat> T interpolateInformation(uint64_t bufferAddress, uint64_t byteStride, uint64_t offset, uint3 index, float3 barycentrics) {
        T attr0 = readBuffer<T>(BufferReadInfo(bufferAddress, byteStride, offset, index.x));
        T attr1 = readBuffer<T>(BufferReadInfo(bufferAddress, byteStride, offset, index.y));
        T attr2 = readBuffer<T>(BufferReadInfo(bufferAddress, byteStride, offset, index.z));
    
        return T(barycentrics.x) * attr0 + T(barycentrics.y) * attr1 + T(barycentrics.z) * attr2;
    }
    
    Triangle getTriangeInformation(
        uint64_t instanceBuffer,
        uint64_t instanceStride,
        uint64_t vertexStride,
        uint32_t meshID,
        uint primitiveID,
        float3 barycentrics) {
        uint64_t vertexBuffer = ((uint64_t *)(instanceBuffer + instanceStride * meshID))[0];
        uint64_t indexBuffer = ((uint64_t *)(instanceBuffer + instanceStride * meshID + 8))[0];
        uint3 indices = ((int3 *)(indexBuffer))[primitiveID];
        
        Triangle tri;
        tri.pos = interpolateInformation<float3>(vertexBuffer, vertexStride, 0, indices, barycentrics);
        tri.normal = interpolateInformation<float3>(vertexBuffer, vertexStride, 12, indices, barycentrics);
        tri.uv = interpolateInformation<float2>(vertexBuffer, vertexStride, 24, indices, barycentrics);
        
        return tri;
    }

    Material getMaterial(uint64_t materialBuf, uint64_t materialStride, uint64_t instanceBuf, uint64_t instanceStride, uint32_t instanceID) {
        uint32_t index = ((uint32_t *)(instanceBuf + instanceStride * instanceID + 16))[0];
        return ((Material*)(materialBuf + materialStride * index))[0]; 
    }
}