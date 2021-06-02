
float3 checkerBoard(float3 viewingRayposition, float backgroundScale)
{
    int3 idx = int3(viewingRayposition / backgroundScale) + 65536;
    if (((idx.x & 0x1) ^ (idx.y & 0x1)) ^ (idx.z & 0x1))
        return float3(0.5, 0.5, 0.5);
    else
        return float3(0.01, 0.01, 0);
}
