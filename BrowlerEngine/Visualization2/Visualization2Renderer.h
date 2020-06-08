#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/AppRenderer.h"
#include "UI/AppUi.h"
#include "Renderer/Renderer.h"
#include "PitImage.h"
#include "DataSet.h"
#include "TextureResource.h"

#ifndef BRWL_USE_DEAR_IM_GUI
#error The project requires Dear ImGui
#else
#include "UI/ImGui/imgui.h"
#endif

#include "Renderer/MainShader.h"



BRWL_RENDERER_NS


class BaseRenderer;

class Visualization2Renderer : public AppRenderer
{
public:
	Visualization2Renderer();

protected:
	virtual bool init(PAL::WinRenderer* renderer) override;
	virtual void preRender(PAL::WinRenderer* renderer) override;
	virtual void render(PAL::WinRenderer* renderer) override;
	virtual void draw(PAL::WinRenderer* renderer) override;
	// We expect that destroy is onyl called when no resources are in use
	virtual void destroy() override;

	void LoadFonts(float fontSize);

	uint8_t uiResultIdx;
	UIResult uiResults[2];
	ImFont* fonts[ENUM_CLASS_TO_NUM(UIResult::Settings::Font::MAX)];

	ComPtr<ID3D12CommandQueue> uploadCommandQueue;
	ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> uploadCommandList;

	// The main data set
	DataSet dataSet;
	TextureResource volumeTexture;
	uint64_t volumeTextureFenceValue;
	// Preintegration table
	PitImage pitImage;

	ComPtr<ID3D12Resource> uploadHeap;
	ComPtr<ID3D12Fence> volumeTextureUploadFence;
	HANDLE uploadFenceEvent;

	MainShader mainShader;

	bool initialized;
};

BRWL_RENDERER_NS_END