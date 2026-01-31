#pragma once

namespace Light {
    enum LightType : uint8_t {
        POINT = 0,
        SPOT = 1,
        DIRECTIONAL = 2,
        AREA = 3
    };

    struct Light {
        float3 position;
        float3 color;
        float3 direction;
        float intensity;
        LightType type;
    };

    Light processLight(uint64_t bufferAddress, uint64_t byteStride, uint32_t ID, float3 worldPos) {
        Light light;
        light.position = ((float3*)(bufferAddress + byteStride * ID))[0];
        light.color = ((float3*)(bufferAddress + byteStride * ID + 12))[0];
        light.intensity = ((float*)(bufferAddress + byteStride * ID + 24))[0];
        light.type = ((LightType*)(bufferAddress + byteStride * ID + 28))[0];

        /*if (light.type == LightType.POINT) {
            light.direction = light.position - worldPos;
            float d = length(light.direction);
            light.intensity /= (d * d);
            return light;
        }

        return light;*/

        light.direction = light.position - worldPos;
        float d = length(light.direction);
        light.intensity /= (d * d);
        return light;
    }
}