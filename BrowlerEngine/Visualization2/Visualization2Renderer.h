#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/AppRenderer.h"
#include "AppUi.h"
#include "Renderer/DataSet.h"
#include "PitCollection.h"
#include "Renderer/RendererFwd.h"
#include "Renderer/TextureHandle.h"

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
	void setFilePath(const BRWL_CHAR* file) { std::scoped_lock(assetPathMutex); assetPath = file; assetPathUpdated = true; }

protected:
	virtual bool init(Renderer* renderer) override;
	virtual void preRender(Renderer* renderer) override;
	virtual void render(Renderer* renderer) override;
	virtual void draw(Renderer* renderer) override;
	// We expect that destroy is onyl called when no resources are in use
	virtual void destroy(Renderer* renderer) override;

	void LoadFonts(float fontSize);
	/**
	 * (Re)load the asset cpu-side data from disk.
	 * 
	 * \param r The Renderer to use
	 * \returns True if the reload succeeded, false if not.
	 */
	bool ReloadVolumeAsset(BRWL::Renderer::Renderer* r);

	uint8_t uiResultIdx;
	UIResult uiResults[2];
	ImFont* fonts[ENUM_CLASS_TO_NUM(UIResult::Settings::Font::MAX)];

	// The main data set
	std::mutex assetPathMutex;
	BRWL_STR assetPath;    // set from different thread
	bool assetPathUpdated; // 

	BaseTextureHandle dataSetHandle;
	PitCollection pitCollection;

	MainShader mainShader;

	bool initialized;
	bool hasViewChanged;
	size_t hasCameraMovedListenerHandle;
};

BRWL_RENDERER_NS_END