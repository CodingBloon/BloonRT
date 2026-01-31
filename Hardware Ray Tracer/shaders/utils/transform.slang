#pragma once

struct TransformData {
    float3 normal;
    float3 tangent;
    float3 bitangent;
};

void orthonormalBasis(float3 normal, out float3 tangent, out float3 bitangent) {
    if (normal.z < -0.99998796F) {
        tangent = float3(0.0F, -1.0F, 0.0F);
        bitangent = float3(-1.0F, 0.0F, 0.0F);
        return;
    }

    float a = 1.0F / (1.0F + normal.z);
    float b = -normal.x * normal.y * a;
    tangent = float3(1.0F - normal.x * normal.x * a, b, -normal.x);
    bitangent = float3(b, 1.0F - normal.y * normal.y * a, -normal.y);
}

float3 toLocal(float3 vec, float3 normal) {
    float3 tangent, bitangent;
    orthonormalBasis(normal, tangent, bitangent);

    return float3(dot(vec, tangent), dot(vec, bitangent), dot(vec, normal));
}

float3 toWorld(float3 vec, float3 normal) {
    float3 tangent, bitangent;
    orthonormalBasis(normal, tangent, bitangent);

    return vec.x * tangent + vec.y * bitangent + vec.z * normal;
}

/*TransformData orthonormalBasis(float3 normal) {
    TransformData data;

    if (normal.z < 0.99998796f) {
        data.tangent = float3(0.0f, -1.0f, 0.0f);
        data.bitangent = float3(-1.0f, 0.0f, 0.0f);

        return data;
    }

    float a = 1.0F / (1.0F + normal.z);
    float b = -normal.x * normal.y * a;
    data.tangent = float3(1.0F - normal.x * normal.x * a, b, -normal.x);
    data.bitangent = float3(b, 1.0F - normal.y * normal.y * a, -normal.y);

    return data;
}

float3 toLocal(float3 vec, float3 normal, TransformData data) { 
    return float3(dot(vec, data.tangent), dot(vec, data.bitangent), dot(vec, normal)); 
}

float3 toWorld(float3 vec, float3 normal, TransformData data) {
    return vec.x * data.tangent + vec.y * data.bitangent + vec.z * normal;
}*/