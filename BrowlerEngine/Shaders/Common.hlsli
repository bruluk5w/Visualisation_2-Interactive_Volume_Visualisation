
float3 checkerBoard(float3 viewingRayDirection)
{
    int2 idx = 10 * (viewingRayDirection / viewingRayDirection.z).xy;
    if ((idx.x & 0x1) ^ (idx.y & 0x1))
        return float3(0.5, 0.5, 0.5) - (0.25 - abs(viewingRayDirection.z * 0.25));
    else
        return float3(0.01, 0.01, 0) + (0.49 - abs(viewingRayDirection.z * 0.49));
}
