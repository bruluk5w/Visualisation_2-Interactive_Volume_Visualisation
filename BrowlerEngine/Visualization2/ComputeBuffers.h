#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS


class ComputeBuffers final
{
public:
	ComputeBuffers();

	bool create(ID3D12Device* device, PAL::DescriptorHeap* srvHeap, unsigned int width, unsigned int height);
	void destroy();
	bool isResident() const;
	// swaps the aliasing in for the compute buffers and the ping pong variable
	void swap(ID3D12GraphicsCommandList* cmd);
	// source handles 0 - numBuffers are guaranteed to be a contiguous range
	PAL::DescriptorHandle::ResidentHandles getSourceSrv(unsigned int idx);
	// target handles 0 - numBuffers are guaranteed to be a contiguous range
	PAL::DescriptorHandle::ResidentHandles getTargetUav(unsigned int idx);
	// source buffer 0 - numBuffers are guaranted to be a contiguous range
	// the resource to read from 
	// if before is true, then returns the resource which the alias referred to before 
	ID3D12Resource* getSrvResource(unsigned int idx, bool before = false);
	// target buffer 0 - numBuffers are guaranted to be a contiguous range
	// the resource to write to
	// if before is true, then returns the resource which the alias referred to before 
	ID3D12Resource* getUavResource(unsigned int idx, bool before = false);
	unsigned int getWidth() const { return bufferWidth; }
	unsigned int getHeight() const { return bufferHeight; }
	// 6 buffers with 2 layers each for ping-pong renderering
	const static unsigned int numBuffers = 6;
	const static unsigned int numLayers = 2;
private:
	unsigned int bufferWidth;
	unsigned int bufferHeight;
	ComPtr<ID3D12Resource> srvBuffers[numLayers * numBuffers];
	ComPtr<ID3D12Resource> uavBuffers[numLayers * numBuffers];

	ComPtr<ID3D12Heap> bufferHeap;
	PAL::DescriptorHandle* srvDescriptorRange;
	PAL::DescriptorHandle* uavDescriptorRange;
	bool created;

	bool pingPong;

};


BRWL_RENDERER_NS_END