#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS


class ComputeBuffers final
{
public:
	ComputeBuffers();

	bool create(ID3D12Device* device, PAL::DescriptorHeap* srvHeap, unsigned int width, unsigned int height);
	// todo: this is kind of a hack. Resource states should probably be tracked by texture manager
	void setInitialResourceState(ID3D12GraphicsCommandList* cmd);

	void destroy(PAL::DescriptorHeap* srvHeap);
	bool isResident() const;
	// to be called before resources are used in the compute shaders
	// swaps the source resources with the target resources and the ping pong variable
	// endes the barrier that ensures that writes to the new source have finished before we read.
	void beforeComputeUse(ID3D12GraphicsCommandList* cmd);
	// to be called after resources were used in the compute shaders
    // starts the barrier that ensures that writes to the current target (next source) have finished before we read.
	void afterComputeUse(ID3D12GraphicsCommandList* cmd);
	
	// target buffer 0 to numBuffers are guaranted to be a contiguous range
	// the resource to write to
	// if before is true, then returns the resource which has been the target before
	ID3D12Resource* getTargetResource(unsigned int idx, bool before = false);
	// source buffer 0 to numBuffers are guaranted to be a contiguous range
	// the resource to read from
	// if before is true, then returns the resource which has been the source before
	// the source resources may not be written to in the shader!
	ID3D12Resource* getSourceResource(unsigned int idx, bool before = false);


	// target uav descriptor 0 - numBuffers are guaranted to be a contiguous descriptor range
	PAL::DescriptorHandle::ResidentHandles getTargetResourceDescriptorHandle(unsigned int idx);
	// source uav descriptor - numBuffers are guaranted to be a contiguous range
	PAL::DescriptorHandle::ResidentHandles getSourceUavResourceDescriptorHandle(unsigned int idx);
	// source srv descriptor 0 to numSrvReadBuffers are guaranted to be a contiguous range
	PAL::DescriptorHandle::ResidentHandles getSourceSrvResourceDescriptorHandle(unsigned int idx);

	unsigned int getWidth() const { return bufferWidth; }
	unsigned int getHeight() const { return bufferHeight; }
	bool isCreated() const { return created; }
	// 6 buffers with 2 layers each for ping-pong renderering
	const static unsigned int numBuffers = 6;
	const static unsigned int numLayers = 2;
	const static unsigned int colorBufferIdx = 2;
	const static unsigned int srvReadBufferOffset = 0;	// signifies the range of bufferes that are transitioned for UAVs to SRVs (and back) when switched to become the read layer
	const static unsigned int numSrvReadBuffers = 2;	//

private:
	void closeUavBarrier(ID3D12GraphicsCommandList* cmd);
	unsigned int bufferWidth;
	unsigned int bufferHeight;
	ComPtr<ID3D12Resource> resources[numLayers * numBuffers];

	ComPtr<ID3D12Heap> bufferHeap;
	PAL::DescriptorHandle* uavDescriptorRange;
	PAL::DescriptorHandle* srvDescriptorRange;
	bool created;

	bool pingPong; // if true then read layer starts at offset 0, else at numBuffers
#if USE_UAV_SPLIT_BARRIER
	bool uavBarrierActive;
#endif
	bool uavUseSet;
	bool colorTexUsedByPixelShader;
};


BRWL_RENDERER_NS_END