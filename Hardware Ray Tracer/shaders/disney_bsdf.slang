#include "material.slang"
#include "constants.slang"

// ==================== UTILITY FUNCTIONS =========================
inline float schlichFresnel(float F0, float F90, float f) { return F0 + (F90 - F0) * pow(1.0f - f, 5.0f); }
inline float square(float f) { return f * f; }
inline float schlickWeight(float f) { float m = clamp(1 - f, 0.0f, 1.0f); return (m * m) * (m * m) * m; }

float GTR1(float absHdotL, float a) {
    if (a >= 1.0f)
        return ONE_OVER_PI;

    float a2 = a * a;
    return (a2 - 1.0f) / (PI * log2(a2) * (1.0f + (a2 - 1.0f) * absHdotL * absHdotL));
}

float separableGGX(float3 vec, float a) {
    float a2 = a * a;
    float absNdotV = abs(0);
}

// ==================== BSDF FUNCTIONS =========================
float3 calculateTint(float3 baseColor) {
    float luminace = dot(float3(0.3f, 0.6f, 1.0f), baseColor);
    return select(luminace > 0.0f, baseColor * (1.0f / luminace), float3(1.0f, 1.0f, 1.0f));
}

//Calculate intensity of the sheen lobe
float3 evaluateSheen(Material *mat, float3 wo, float3 wm, float3 wi) {
    if (mat.sheen <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    float hdotL = dot(wm, wi);
    float3 tint = calculateTint(mat.color);
    return mat.sheen * lerp(float3(1.0f), tint, mat.sheenTint) * schlickWeight(hdotL);
}

void evaluateClearcoat() {}
void evaluateBRDF() {}
void evaluateSpecTransmission() {}
void evaluateDiffuse() {}
void evaluateBSDF() {}

// ==================== HEMISPHERE SAMPLING FUNCTIONS ====================
void sampleHemisphere() {}