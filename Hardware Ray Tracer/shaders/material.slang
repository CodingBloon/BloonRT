#pragma once

struct Material {
    float3 color;
    float metallic;
    float specualar;
    float roughness;
    float specualarTint;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearCoat;
    float clearCoatGloss;
    float specualarTransmission;
};