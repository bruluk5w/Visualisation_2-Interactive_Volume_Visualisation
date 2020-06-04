#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/AppRenderer.h"
#include "UI/AppUi.h"
#include "DataSet.h"

#ifndef BRWL_USE_DEAR_IM_GUI
#error The project requires Dear ImGui
#else
#include "UI/ImGui/imgui.h"
#endif


BRWL_RENDERER_NS


class BaseRenderer;

class Visualization2Renderer : public AppRenderer
{
public:
	Visualization2Renderer();
protected:
	virtual bool init() override;
	virtual void preRender() override;
	virtual void render() override;
	virtual void draw(PAL::WinRenderer* renderer) override;
	virtual void destroy() override;

	void LoadFonts(float fontSize);

	uint8_t uiResultIdx;
	UIResult uiResults[2];
	ImFont* fonts[ENUM_CLASS_TO_NUM(UIResult::Font::MAX)];

	DataSet dataSet;
};

BRWL_RENDERER_NS_END