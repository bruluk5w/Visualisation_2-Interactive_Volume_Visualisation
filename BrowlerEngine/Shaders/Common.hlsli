#include "ShaderDefines.h"

float3 checkerBoard(float3 viewingRayposition, float backgroundScale)
{
    int3 idx = int3(viewingRayposition / backgroundScale + 65536) ;
    if (((idx.x & 0x1) ^ (idx.y & 0x1)) ^ (idx.z & 0x1))
        return float3(0.5, 0.5, 0.5);
    else
        return float3(0.01, 0.01, 0);
}

bool rayIntersectsVolume(float3 rayPos, float3 rayDir, float3 bboxmax, float3 bboxmin)
{
    float xdistancemax = bboxmax.x - rayPos.x;
    // (xdistancemax * rayDir > 0) checks if they have the same preceding sign (pos/neg)
    bool intersectsPositiveX = (xdistancemax * rayDir.x > 0) && all(abs((rayPos + rayDir * (xdistancemax / rayDir.x)).yz) <= bboxmax.yz);
    float xdistancemin = bboxmin.x - rayPos.x;
    bool intersectsNegativeX = (xdistancemin * rayDir.x > 0) && all(abs((rayPos + rayDir * (xdistancemin / rayDir.x)).yz) <= bboxmax.yz);
    float ydistancemax = bboxmax.y - rayPos.y;
    bool intersectsPositiveY = (ydistancemax * rayDir.y > 0) && all(abs((rayPos + rayDir * (ydistancemax / rayDir.y)).xz) <= bboxmax.xz);
    float ydistancemin = bboxmin.y - rayPos.y;
    bool intersectsNegativeY = (ydistancemin * rayDir.y > 0) && all(abs((rayPos + rayDir * (ydistancemin / rayDir.y)).xz) <= bboxmax.xz);
    float zdistancemax = bboxmax.z - rayPos.z;
    bool intersectsPositiveZ = (zdistancemax * rayDir.z > 0) && all(abs((rayPos + rayDir * (zdistancemax / rayDir.z)).xy) <= bboxmax.xy);
    float zdistancemin = bboxmin.z - rayPos.z;
    bool intersectsNegativeZ = (zdistancemin * rayDir.z > 0) && all(abs((rayPos + rayDir * (zdistancemin / rayDir.z)).xy) <= bboxmax.xy);
    
    return (intersectsNegativeX || intersectsNegativeY || intersectsNegativeZ ||
            intersectsPositiveX || intersectsPositiveY || intersectsPositiveZ);
}

float3 blinnPhongSpecular(float3 lightDir, float3 viewDir, float3 normal, float3 lightIntensity, float reflectivity)
{
    const float3 halfVector = normalize(lightDir + viewDir);
    return pow(max(dot(normal, halfVector), 0.00001), reflectivity) * lightIntensity;
}

float quadrilateralArea(float3 a, float3 b, float3 c, float3 d)
{
    return (length(cross(b - a, d - a)) + length(cross(b - c, d - c))) * 0.5f;
}

