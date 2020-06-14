#include "AppUi.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_plot.h"
#include "Common/Spline.h"
#include <algorithm>


BRWL_NS


const char* UIResult::Settings::fontNames[] = {
    "Open Sans Light",
    "Open Sans Regular",
    "Open Sans Semibold",
    "Open Sans Bold"
};

const char* UIResult::TransferFunction::bitDepthNames[] = {
    "8 Bit",
    "10 Bit"
};

const char* UIResult::TransferFunctionCollection::transferFuncNames[] = {
    "Refraction",
    "Particle Color",
    "Opacity",
    "Medium Color"
};

UIResult::UIResult() :
    settings {
        UIResult::Settings::Font::OPEN_SANS_REGULAR, // font
        30, // fontSize
        300, // voxelsPerCm
    },
    transferFunctions{}
{ }


UIResult::TransferFunctionCollection::TransferFunctionCollection() :
    functions {
        {}, // refractionTansFunc
        {}, // particleColorTransFunc
        {}, // opacityTransFunc
        {}  // mediumColorTransFunc
    }
{ }

UIResult::TransferFunctionCollection::~TransferFunctionCollection()
{
    functions.~Aliases();
}

UIResult::TransferFunction::TransferFunction()  :
    bitDepth(UIResult::TransferFunction::BitDepth::BIT_DEPTH_8_BIT),
    controlPoints{ {0,0}, {1,1} },
    transferFunction { 0 },
    textureID(nullptr)
{
    updateFunction();
}


int UIResult::TransferFunction::getArrayLength() const
{
    switch (bitDepth) {
    case BitDepth::BIT_DEPTH_8_BIT:
        return 1 << 8;
    case BitDepth::BIT_DEPTH_10_BIT:
        return 1 << 10;
    default:
        BRWL_UNREACHABLE();
    }

    return 0;
}

void UIResult::TransferFunction::updateFunction()
{
    const int numPoints = (int)controlPoints.size();
    if (!BRWL_VERIFY(numPoints >= 2, nullptr)) return;

    // evaluate the spline at non-uniform locations
    if (!BRWL_VERIFY(controlPoints[0].x == 0 && controlPoints[numPoints - 1].x == 1, nullptr)) {
        return;
    }
    Vec2 first(controlPoints[0] - (controlPoints[1] - controlPoints[0]));
    Vec2 last(controlPoints[numPoints - 1] + (controlPoints[numPoints - 1] - controlPoints[numPoints - 2]));
    const int numSamples = getArrayLength();
    const int numSubdivisions = 30;
    const int numTessellatedPoints = (numPoints - 1) * numSubdivisions + 1;
    std::unique_ptr<Vec2[]> tessellatedPoints = std::unique_ptr<Vec2[]>(new Vec2[numTessellatedPoints]);
    Splines::CentripetalCatmullRom(first, controlPoints.data(), last, controlPoints.size(), tessellatedPoints.get(), numSubdivisions, numTessellatedPoints);

    // Interpolate along segments for approximation of uniform intervals along the x axis
    int cursor = 1;
    transferFunction[0] = tessellatedPoints[0].y;
    for (int i = 1; i < numSamples; ++i)
    {
        while (tessellatedPoints[cursor].x * numSamples <= i && cursor < numTessellatedPoints - 1)
        {
            ++cursor;
        }
        const Vec2& previous = tessellatedPoints[cursor - 1];
        const Vec2& next = tessellatedPoints[cursor];
        const Vec2 delta = next - previous;

        transferFunction[i] = previous.y + ((float)i / (float)numSamples - previous.x) * delta.y / delta.x;
    }

    transferFunction[numSamples - 1] = tessellatedPoints[numTessellatedPoints - 1].y;

}

using namespace ImGui;

#define ENUM_SELECT(label, current_val, result_val, scope, enum_name, name_array) \
{ \
    int item_current = ENUM_CLASS_TO_NUM(current_val); \
    ::ImGui::Text(label); SameLine(); \
    ::ImGui::Combo("", &item_current, scope::name_array, IM_ARRAYSIZE(scope::name_array)); \
    result_val = (scope::enum_name)Utils::clamp<std::underlying_type_t<scope::enum_name>>(item_current, ENUM_CLASS_TO_NUM(scope::enum_name::MIN), ENUM_CLASS_TO_NUM(scope::enum_name::MAX)); \
}

#define SLIDER_FIX(id) ImGui::BeginChild(#id, ImVec2(ImGui::GetWindowWidth(), ImGui::GetTextLineHeightWithSpacing()), false);
#define SLIDER_FIX_END() EndChild();

void drawHints();

void renderAppUI(UIResult& result, const UIResult& values)
{
#ifdef BRWL_USE_DEAR_IM_GUI
    drawHints();

    ImGui::BeginMainMenuBar();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    Dummy(ImVec2(20, 0));
    thread_local bool showSettings = false;
    showSettings = showSettings || Button("Settings");
    thread_local bool showTools = false;
    showTools = showTools || Button("Tools");
    ImGui::EndMainMenuBar();

    if (showSettings) {
        Begin("Settings", &showSettings);
        ENUM_SELECT("GUI Font: ", values.settings.font, result.settings.font, UIResult::Settings, Font, fontNames);

        // work around for slider bug where slider is activated when clicking on the combo box above
        SLIDER_FIX(0)
            Text("Font Size:"); SameLine();
            SliderFloat("", &result.settings.fontSize, 5, 40);
        SLIDER_FIX_END();
        Text("Voxels per Centimeter:"); SameLine(); ::ImGui::InputFloat("", &result.settings.voxelsPerCm, 1.f, 5.f);
        End();
    }

    if (showTools)
    {
        thread_local float plotWidth = 1;
        thread_local float plotHeight = 1;
        thread_local float menuSpaceY = 0; // the height of all the stuff before the graph plot comes
        thread_local bool fitWindow = false;

        {
            int arrayLen = result.transferFunctions.array[0].getArrayLength();
            const float minWindowSizeX = arrayLen * plotWidth + 25.f;
            const float minWindowSizeY = Utils::min(GetIO().DisplaySize.y - 20, menuSpaceY + 20);
            if (fitWindow) {
                ImGui::SetNextWindowSize({ minWindowSizeX + 10, minWindowSizeY + 10 });
                fitWindow = false;
            }

            PushStyleVar(ImGuiStyleVar_WindowMinSize, { minWindowSizeX, minWindowSizeY });
        }

        Begin("Tools", &showTools);
        {
            UIResult::TransferFunction::BitDepth resultBitDepth;
            ENUM_SELECT("Bit Depth", values.transferFunctions.array[0].bitDepth, resultBitDepth, UIResult::TransferFunction, BitDepth, bitDepthNames);
            for (int i = 0; i < countof(result.transferFunctions.array); ++i)
            {
                result.transferFunctions.array[i].bitDepth = resultBitDepth;
            }

            thread_local bool lockAspect = false;
            float plotWidthBefore = plotWidth;
            float plotHeihgtBefore = plotHeight;
            // Draw first plot with multiple sources
            Text("Plot Size: ");
            SLIDER_FIX(1);
            Text("Width: "); SameLine(0, 10); int cursorX = GetCursorPosX(); SliderFloat("", &plotWidth, 1, 3, "");
            if (!lockAspect) { SameLine(); if (Button("Reset")) plotWidth = 1; }
            SLIDER_FIX_END();
            SLIDER_FIX(2);
            Text("Height: "); SameLine(cursorX); SliderFloat("", &plotHeight, 1, 3, "");
            if (!lockAspect) { SameLine(); if (Button("Reset")) plotHeight = 1; }
            SLIDER_FIX_END();
            fitWindow = Button(" Fit Window");
            if (lockAspect) {
                if (plotWidth != plotWidthBefore) plotHeight = plotWidth;
                else if (plotHeight != plotHeihgtBefore) plotWidth = plotHeight;
                else if (plotHeight != plotWidth) plotHeight = plotWidth = Utils::max(plotHeight, plotWidth);
            }
            
            Checkbox(" Maintain Aspect Ratio", &lockAspect);
            if (lockAspect) { SameLine(); if (Button("Reset")) plotWidth = plotHeight = 1; }
            thread_local bool showCoordinatesTooltip = true;
            Checkbox(" Show Tooltip in Graph", &showCoordinatesTooltip); 
            thread_local float ctrlPtointScale = 1;
            SLIDER_FIX(3);
            Text("Control Point Size: "), SameLine(); SliderFloat("", &ctrlPtointScale, 0.2f, 3.f);
            SLIDER_FIX_END();
            
            // Tab Bar
            //thread_local bool opened[ENUM_CLASS_TO_NUM(UIResult::TransferFuncType::MAX)] = { true, true, true, true };

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
            // the underlying bool will be set to false when the tab is closed.
            if (ImGui::BeginTabBar("Transfer Functions", 
                ImGuiTabBarFlags_FittingPolicyResizeDown | 
                //ImGuiTabBarFlags_Reorderable | 
                ImGuiTabBarFlags_AutoSelectNewTabs 
                //ImGuiTabBarFlags_TabListPopupButton
            ))
            {
                for (int i = 0; i < ENUM_CLASS_TO_NUM(UIResult::TransferFuncType::MAX); i++)
                    if (ImGui::BeginTabItem(UIResult::TransferFunctionCollection::transferFuncNames[i], nullptr, ImGuiTabItemFlags_None))
                    {
                        Text(UIResult::TransferFunctionCollection::transferFuncNames[i]);
                        const UIResult::TransferFunction& valueTFunc = values.transferFunctions.array[i];
                        UIResult::TransferFunction& resultTFunc = result.transferFunctions.array[i];

                        bool ctrlPointsChanged = false;
                        ImGui::PlotConfig conf{ };
                        conf.values.count = resultTFunc.getArrayLength();
                        conf.values.ys = resultTFunc.transferFunction; // use ys_list to draw several lines simultaneously
                        conf.values.ys_count = 1;
                        const ImU32 graphColor(ImColor(0, 255, 0));
                        conf.values.colors = &graphColor;
                        conf.scale.min = 0;
                        conf.scale.max = 1;
                        conf.tooltip.show = showCoordinatesTooltip;
                        conf.tooltip.format = "Idx: %g, Cursor Value: %8.4g, Graph Value %8.4g";
                        conf.grid_x.show = true;
                        conf.grid_x.size = 128;
                        conf.grid_x.subticks = 4;
                        conf.grid_y.show = true;
                        conf.grid_y.size = 1.0f;
                        conf.grid_y.subticks = 5;
                        conf.selection.show = false;
                        conf.ctrlPoints = &valueTFunc.controlPoints;
                        conf.ctrlPointSize = ctrlPtointScale;
                        conf.values.ctrlPoints = &resultTFunc.controlPoints;
                        conf.values.ctrlPointsChanged = &ctrlPointsChanged;
                        //conf.selection.start = &selection_start;
                        //conf.selection.length = &selection_length;
                        int texSideLength = valueTFunc.getArrayLength();
                        float width = texSideLength * plotWidth;
                        float height = texSideLength * plotHeight;
                        conf.frame_size = ImVec2(width, height);
                        if (valueTFunc.textureID != nullptr)
                        {
                            conf.useBackGroundTextrue = true;
                            conf.texID = valueTFunc.textureID;
                            conf.maxTexVal = 1;// (float)texSideLength;
                        }

                        ImGui::Plot("plot1", conf);
                        ctrlPointsChanged |= valueTFunc.bitDepth != resultTFunc.bitDepth;
                        if (ctrlPointsChanged)
                        {
                            decltype(resultTFunc.controlPoints)& array = resultTFunc.controlPoints;
                            std::sort(array.begin(), array.end(), [](const Vec2& a, const Vec2& b) {return a.x < b.x; });
                            if (array.size() >= 2)
                            {
                                // remove duplicates
                                array.erase(std::remove_if(array.begin() + 1, array.end(), [&array](::BRWL::Vec2& v){return v.x == array[&v - array.data() - 1].x; }), array.end());
                            }

                            resultTFunc.updateFunction();
                        }

                        ImGui::EndTabItem();
                    }
                ImGui::EndTabBar();
            }
        }

        // remember Y position for window fit
        menuSpaceY = ImGui::GetCursorPosY();

        End(); // Tools
        PopStyleVar();
    }

#endif // BRWL_USE_DEAR_IM_GUI

}

#ifdef BRWL_USE_DEAR_IM_GUI

void drawHints()
{
    const ImGuiIO& io = GetIO();
    //io.DisplaySize
    ImGuiWindowFlags dummyFlags = 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoSavedSettings | 
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_AlwaysAutoResize;
    if (!io.WantCaptureMouse) {
        if (io.MouseDown[0]) {
            if (!IsKeyDown(io.KeyMap[ImGuiKey_Space])) {
                SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                Begin("HintWindow", nullptr, dummyFlags);
                Text("Rotate FPS view: <SPACE> + <MOUSE LEFT>");
                End();
            }
            else
            {
                SetNextWindowPos(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always, ImVec2(1.f, 1.f));
                Begin("HintWindow", nullptr, dummyFlags);
                Text("Strave: <WSAD> | Up: <E> | Down: <Q> ");
                End();
            }
        }
    }
}

#endif // BRWL_USE_DEAR_IM_GUI


BRWL_NS_END
