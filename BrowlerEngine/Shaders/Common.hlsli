
float3 checkerBoard(float3 viewingRayDirection)
{
    int2 idx = 10 * (viewingRayDirection / viewingRayDirection.z).xy;
    if ((idx.x & 0x1) ^ (idx.y & 0x1))
        return float3(1, 1, 1) - (0.5 - abs(viewingRayDirection.z * 0.5));
    else
        return float3(0, 0, 0) + (0.5 - abs(viewingRayDirection.z * 0.5));
}
