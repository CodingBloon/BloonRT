#pragma once

struct UniformBuffer {
    float4x4 viewInverse;
    float4x4 projInverse;
    uint32_t frameIndex;
    uint32_t depthMax;
    float LIGHT_THRESHOLD;
};

struct SceneBuffer {
    uint64_t materialBuffer;
    uint64_t materialByteStride;
    uint64_t lightBuffer;
    uint64_t lightByteStride;
    uint64_t numLights;
    uint64_t vertexByteStride;
    uint64_t instanceBuffer;
    uint64_t instanceByteStride;
    uint64_t skyBuffer;
    uint64_t skyStride;
};

struct BufferReadInfo {
    uint64_t address;
    uint64_t stride;
    uint64_t offset = 0;
    uint32_t element;
};

BufferReadInfo buildBufferReadInfo(uint64_t address, uint64_t stride, uint64_t offset, uint32_t index) {
    BufferReadInfo info;
    info.address = address;
    info.stride = stride;
    info.offset = offset;
    info.element = index;

    return info;
}

__generic<T> T readBuffer(BufferReadInfo info) {
    T *ptr = (T *)(info.address + info.offset + info.stride * info.element);
    return ptr[0];
}