#include "Visualization2Renderer.h"

#include "Core/BrowlerEngine.h"
#include "Common/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/PAL/imgui_impl_dx12.h"
#include <mutex>

BRWL_RENDERER_NS


Visualization2Renderer::Visualization2Renderer() :
    AppRenderer(),
    uiResultIdx(0),
    fonts{ 0 },
    dataSet(BRWL_CHAR_LITERAL("Default Data Sets (Beetle)"))
{
    uiResults[0] = { UIResult::Font::OPEN_SANS_REGULAR, 30 };
    uiResults[1] = uiResults[0];
}

bool Visualization2Renderer::init()
{
    LoadFonts(uiResults[uiResultIdx].fontSize);
    if (!dataSet.isValid())
    {
        engine->logger->info(BRWL_CHAR_LITERAL("Loading the beatle asset."));
        dataSet.loadFromFile(BRWL_CHAR_LITERAL("./Assets/DataSets/stagbeetle832x832x494.dat"));
        BRWL_EXCEPTION(dataSet.isValid(), BRWL_CHAR_LITERAL("Failed to load default asset."));
    }

    return true;
}

void Visualization2Renderer::preRender()
{
    const UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    if (v.fontSize != r.fontSize) {
        LoadFonts(r.fontSize);
        v.fontSize = r.fontSize;
    }
}

void Visualization2Renderer::render()
{
    uiResultIdx = uiResultIdx ? 0 : 1;
    UIResult& r = uiResults[uiResultIdx]; // results
    const UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    ImGui::PushFont(fonts[ENUM_CLASS_TO_NUM(r.font)]);
    renderAppUI(r, v);
    ImGui::PopFont();
}
void Visualization2Renderer::draw(Renderer* r)
{
    
}

void Visualization2Renderer::destroy()
{
}

void Visualization2Renderer::LoadFonts(float fontSize)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    fonts[0] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Light.ttf", fontSize);
    fonts[1] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Regular.ttf", fontSize);
    fonts[2] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Semibold.ttf", fontSize);
    fonts[3] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Bold.ttf", fontSize);
    ImGui_ImplDX12_CreateFontsTexture();
}


BRWL_RENDERER_NS_END
