#pragma once
#include "utils/constants.slang"
#include "utils/mesh.slang"
#include "utils/transform.slang"
#include "utils/random.slang"

inline float schlickFresnel(float F0, float VdotH) { return F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f); }
inline float schlickWeight(float f) { float m = clamp(1.0f - f, 0.0f, 1.0f); return m * m * m * m * m; } // pow(m, 5.0f)
inline float square(float f) { return f * f; }

float GTR1(float NdotH, float a) {
    if (a >= 1.0f) return ONE_OVER_PI;
    float a2 = a * a;
    return (a2 - 1.0f) / (PI * log2(a2) * (1.0f + (a2 - 1.0f) * NdotH * NdotH));
}

float GTR2(float NdotH, float a) {
    float a2 = a * a;
    return a2 / (PI * (1.0f + (a2 - 1.0f) * NdotH * NdotH));
}

float GTR2_anisotropic(float NdotH, float HdotX, float HdotY, float2 a) {
    return 1 / (PI * a.x * a.y * square(square(HdotX / a.x) + square(HdotY / a.y) + NdotH * NdotH));
}

float GGX(float NdotV, float a) {
    float a2 = a * a;
    return 2.0f / (1.0f + sqrt(a2 + (1.0f - a2) * NdotV * NdotV));
}

float GGX_anisotropic(float NdotV, float VdotX, float VdotY, float2 a) {
    return 1 / (NdotV + sqrt(square(VdotX * a.x) + square(VdotY * a.y) * NdotV * NdotV));
}

float3 calculateTint(float3 color) {
    float l = dot(float3(0.3f, 0.6f, 1.0f), color);
    return select(l > 0.0f, color * (1.0f / l), float3(1.0f));
}

float2 calculateAnisotropicParameters(Mesh::Material *material) {
    float aspect = sqrt(1.0f - material.anisotropic * 0.9f);
    float r2 = square(material.roughness);
    return float2(
        max(0.001f, r2 / aspect),
        max(0.001f, r2 * aspect));
}

namespace BRDF {
    float3 evalSheen(Mesh::Material *material, float HdotL) {
        float3 tint = calculateTint(material.color);
        return lerp(float3(1.0f), tint, material.sheenTint) * schlickWeight(HdotL);
    }

    float3 evalClearcoat(Mesh::Material *material, float NdotH, float NdotL, float NdotV, float LdotH) {
        float d = GTR1(NdotH, lerp(0.1f, 0.001f, material.clearCoatGloss));
        float f = schlickFresnel(0.04f, LdotH);
        float g = GGX(NdotL, 0.25f) * GGX(NdotV, 0.25f);

        return 0.25f * material.clearCoat * d * f * g;
    }

    float evalDiffuse(Mesh::Material *material, float3 N, float3 L, float3 V, float3 H) {
        float FL = schlickWeight(L.z);
        float FV = schlickWeight(V.z);

        float FD90 = 0.5 + 2 * material.roughness * square(dot(H, L));
        float FD = lerp(1.0f, FD90, FL) * lerp(1.0f, FD90, FV);

        float Fss90 = square(dot(L, H)) * material.roughness;
        float Fss = lerp(1.0f, Fss90, FL) * lerp(1.0f, Fss90, FV);
        float ss = 1.25f * (Fss * (1 / (L.z + V.z) - 0.5f) + 0.5f);

        return lerp(FD, ss, material.subsurface);
    }

    float3 evalSpecular(Mesh::Material *material, float NdotH, float NdotL, float NdotV, float3 H, float3 V, float3 L) {
        float2 anisotropicParameters = calculateAnisotropicParameters(material);

        float3 tint = calculateTint(material.color);
        float3 color = lerp(material.specular * 0.08f * lerp(float3(1.0f), tint, material.specularTint), material.color, material.metallic);

        float d = GTR2_anisotropic(NdotH, H.x, H.y, anisotropicParameters);
        float fresnel = schlickWeight(dot(L, H));
        float3 f = lerp(color, float3(1.0f), fresnel);
        float g = GGX_anisotropic(L.z, L.x, L.y, anisotropicParameters) * GGX_anisotropic(V.z, V.x, V.y, anisotropicParameters);

        return d * f * g;
    }

    float3 BRDF(Mesh::Material *material, float3 N, float3 V, float3 L) {
        float NdotL = dot(N, L);
        float NdotV = dot(N, V);

        if (NdotL <= 0.0f || NdotV <= 0.0f)
            return float3(0.0f);

        float3 H = normalize(V + L);
        float NdotH = dot(N, H);
        float HdotL = dot(H, L);

        // TransformData transData = orthonormalBasis(N);

        float3 localH = toLocal(H, N);
        float3 localV = toLocal(V, N);
        float3 localL = toLocal(L, N);

        float3 sheen = evalSheen(material, HdotL);
        float3 clearCoat = evalClearcoat(material, NdotH, NdotL, NdotV, HdotL);
        float3 specular = evalSpecular(material, NdotH, NdotL, NdotV, localH, localV, localL);
        float diffuse = evalDiffuse(material, N, localL, localV, localH);

        return (ONE_OVER_PI * diffuse * material.color + sheen) * (1 - material.metallic) + specular + clearCoat;
    }

    float reflectivity(Mesh::Material *material) { return 1.0f; }
};

// Importance Sampling for the Disney BRDF
namespace Sampling {
    float specularProbability(Mesh::Material *material, float3 wo) {
        float3 diffuse = material.color * (1.0f - material.metallic);
        float3 F0 = lerp(0.04f, material.color, material.metallic);
        float3 fresnel = schlickWeight(wo.z);
        float3 specColor = lerp(F0, float3(1.0f), fresnel);

        float lumDiffuse = dot(float3(0.3f, 0.6f, 1.0f), diffuse);
        float lumSpecular = dot(float3(0.3f, 0.6f, 1.0f), specColor);

        return lumSpecular / (lumDiffuse + lumSpecular);
    }

    float3 sample_surface(Mesh::Material *material, float3 N, float3 V, inout uint seed, out float pdf) {
        float2 anisotropic = calculateAnisotropicParameters(material);
        //TransformData transData = orthonormalBasis(N);
        float3 wo = toLocal(V, N);
        float r = rand(seed);
        float2 rand = float2(rand(seed), rand(seed));
        float specProb = specularProbability(material, wo);

        if (r < specProb) {
            float3 wm = microfacet_ggx_sample_vndf(wo, anisotropic, rand);
            float3 wi = reflect(wo, wm);
            //divide the pdf by the specular probability (the max is a safety mechanism to prevent a division by zero)
            pdf = microfacet_ggx_vndf_pdf(material, wo, wm, wi) / max(ZERO_TRESHOLD, specProb);
            return toWorld(wi, N);
        }

        float3 wi = lambertain_sample(rand);
        pdf = lambertain_pdf(wi.z) / max(ZERO_TRESHOLD, 1.0f - specProb);
        return toWorld(wi, N);
    }

    float lambertain_pdf(float cosTheta) { return ONE_OVER_PI * cosTheta; }
    float3 lambertain_sample(float2 rand) {
        float phi = TWO_PI * rand.x;
        float cosTheta = sqrt(rand.y);
        float sinTheta = sqrt(max(0.0f, 1.0f - square(cosTheta)));

        return float3(
            sinTheta * cos(phi),
            sinTheta * sin(phi),
            cosTheta
        );
    }

    float microfacet_ggx_vndf_pdf(Mesh::Material *material, float3 wo, float3 wm, float3 wi) {
        if (wi.z < 0.0f) //checks if the incoming direction is going under the surface
            return 0.0f; //pdf is zero as this is impossible (this leads to the termination of the current path)

        // this is a simplified version of the pdf because some terms cancel themselves out with the Disney BRDF
        // return schlickWeight(dot(wi, wm)) * GTR1(dot(wi, wm), square(material.roughness));
        return 1.0f / (schlickWeight(wi.z) * GTR1(wi.z, square(material.roughness)));
    }

    float3 microfacet_ggx_sample_vndf(float3 wo, const float2 anisotropic, const float2 rand) {
        const float3 wo_ = normalize(float3(anisotropic * wo.xy, wo.z));

        float lensq = square(wo_.x) + square(wo_.z);
        float3 T1 = select(lensq > ZERO_TRESHOLD, float3(-wo_.y, wo_.x, 0.0f) * rsqrt(lensq), float3(1.0f, 0.0f, 0.0f));
        float3 T2 = select(lensq > ZERO_TRESHOLD, cross(wo_, T1), float3(0.0f, 1.0f, 0.0f));

        float r = sqrt(rand.x);
        float phi = TWO_PI * rand.y;
        float p1 = r * cos(phi);
        float p2 = r * sin(phi);
        float s = 0.5f * (1.0f + wo_.z);
        p2 = (1.0f - s) * sqrt(1.0f - square(p1)) + s * p2;

        float3 n = T1 * p1 + T2 * p2 + sqrt(max(0.0f, 1.0f - square(p1) - square(p2)));
        return normalize(float3(anisotropic * n.xy, max(0.0f, n.z)));
    }
}