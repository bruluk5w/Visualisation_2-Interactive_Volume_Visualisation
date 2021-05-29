#pragma once // (c) 2021 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS

namespace PAL
{
    
    
    enum class Split : uint8_t
    {
        NONE = 0,
        BEGIN,
        END,
        MAX,
        MIN = 0
    };

    template<D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, Split splitType = Split::NONE, size_t length>
    void stateTransition(ID3D12GraphicsCommandList* cmd, ID3D12Resource* (&resources)[length])
    {
        D3D12_RESOURCE_BARRIER barriers[length];
        memset(&barriers, 0, sizeof(barriers));
        for (int i = 0; i < length; ++i)
        {
            barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[i].Transition.pResource = resources[i];
            barriers[i].Transition.StateBefore = before;
            barriers[i].Transition.StateAfter = after;
            barriers[i].Transition.Subresource = 0;
            if constexpr (splitType != Split::NONE)
                barriers[i].Flags = splitType == Split::BEGIN ? D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY : D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
        }

        cmd->ResourceBarrier((unsigned int)countof(barriers), barriers);
    }
    
    template<D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, Split splitType = Split::NONE, size_t length>
    void stateTransition(ID3D12GraphicsCommandList* cmd, ComPtr<ID3D12Resource> (&resources)[length])
    {
        D3D12_RESOURCE_BARRIER barriers[length];
        memset(&barriers, 0, sizeof(barriers));
        for (int i = 0; i < length; ++i)
        {
            barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[i].Transition.pResource = resources[i].Get();
            barriers[i].Transition.StateBefore = before;
            barriers[i].Transition.StateAfter = after;
            barriers[i].Transition.Subresource = 0;
            if constexpr (splitType != Split::NONE)
                barriers[i].Flags = splitType == Split::BEGIN ? D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY : D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
        }

        cmd->ResourceBarrier((unsigned int)countof(barriers), barriers);
    }


}

BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS
