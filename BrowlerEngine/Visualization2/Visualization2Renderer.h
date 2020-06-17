#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/AppRenderer.h"
#include "UI/AppUi.h"
#include "TextureResource.h"
#include "DataSet.h"
#include "PitCollection.h"
#include "Renderer/RendererFwd.h"

#ifndef BRWL_USE_DEAR_IM_GUI
#error The project requires Dear ImGui
#else
#include "UI/ImGui/imgui.h"
#endif

#include "MainShader.h"



BRWL_RENDERER_NS

class Visualization2Renderer : public AppRenderer
{
public:
	Visualization2Renderer();

protected:
	virtual bool init(Renderer* renderer) override;
	virtual void preRender(Renderer* renderer) override;
	virtual void render(Renderer* renderer) override;
	virtual void draw(Renderer* renderer) override;
	// We expect that destroy is onyl called when no resources are in use
	virtual void destroy(Renderer* renderer) override;

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

	PitCollection pitCollection;

	ComPtr<ID3D12Fence> volumeTextureUploadFence;
	HANDLE uploadFenceEvent;

	MainShader mainShader;

	bool initialized;
	bool skipFrame;
};

BRWL_RENDERER_NS_END