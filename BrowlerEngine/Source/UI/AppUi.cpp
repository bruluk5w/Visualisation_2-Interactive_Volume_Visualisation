#include "AppUi.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_plot.h"
#include "Common/Spline.h"
#include <algorithm>

//#define IMGUI_DEFINE_MATH_OPERATORS

//#include <ImGui/imgui_internal.h>
//#include <time.h>

//namespace ImGui
//{
//    template<int steps>
//    void bezier_table(ImVec2 P[4], ImVec2 results[steps + 1]) {
//        static float C[(steps + 1) * 4], * K = 0;
//        if (!K) {
//            K = C;
//            for (unsigned step = 0; step <= steps; ++step) {
//                float t = (float)step / (float)steps;
//                C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);   // * P0
//                C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t; // * P1
//                C[step * 4 + 2] = 3 * (1 - t) * t * t;     // * P2
//                C[step * 4 + 3] = t * t * t;               // * P3
//            }
//        }
//        for (unsigned step = 0; step <= steps; ++step) {
//            ImVec2 point = {
//                K[step * 4 + 0] * P[0].x + K[step * 4 + 1] * P[1].x + K[step * 4 + 2] * P[2].x + K[step * 4 + 3] * P[3].x,
//                K[step * 4 + 0] * P[0].y + K[step * 4 + 1] * P[1].y + K[step * 4 + 2] * P[2].y + K[step * 4 + 3] * P[3].y
//            };
//            results[step] = point;
//        }
//    }
//
//    float BezierValue(float dt01, float P[4]) {
//        enum { STEPS = 256 };
//        ImVec2 Q[4] = { { 0, 0 }, { P[0], P[1] }, { P[2], P[3] }, { 1, 1 } };
//        ImVec2 results[STEPS + 1];
//        bezier_table<STEPS>(Q, results);
//        return results[(int)((dt01 < 0 ? 0 : dt01 > 1 ? 1 : dt01) * STEPS)].y;
//    }
//
//    int Bezier(const char* label, float P[5]) {
//        // visuals
//        enum { SMOOTHNESS = 64 }; // curve smoothness: the higher number of segments, the smoother curve
//        enum { CURVE_WIDTH = 4 }; // main curved line width
//        enum { LINE_WIDTH = 1 }; // handlers: small lines width
//        enum { GRAB_RADIUS = 8 }; // handlers: circle radius
//        enum { GRAB_BORDER = 2 }; // handlers: circle border width
//        enum { AREA_CONSTRAINED = false }; // should grabbers be constrained to grid area?
//        enum { AREA_WIDTH = 128 }; // area width in pixels. 0 for adaptive size (will use max avail width)
//
//        // bezier widget
//
//        const ImGuiStyle& Style = GetStyle();
//        const ImGuiIO& IO = GetIO();
//        ImDrawList* DrawList = GetWindowDrawList();
//        ImGuiWindow* Window = GetCurrentWindow();
//        if (Window->SkipItems)
//            return false;
//
//        // header and spacing
//        int hovered = IsItemActive() || IsItemHovered(); 
//
//        // eval curve
//        ImVec2 Q[4] = { { 0, 0 }, { P[0], P[1] }, { P[2], P[3] }, { 1, 1 } };
//        ImVec2 results[SMOOTHNESS + 1];
//        bezier_table<SMOOTHNESS>(Q, results);
//
//        //// control points: 2 lines and 2 circles
//        //{
//        //    // handle grabbers
//        //    ImVec2 mouse = GetIO().MousePos, pos[2];
//        //    float distance[2];
//
//        //    for (int i = 0; i < 2; ++i) {
//        //        pos[i] = ImVec2(P[i * 2 + 0], 1 - P[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
//        //        distance[i] = (pos[i].x - mouse.x) * (pos[i].x - mouse.x) + (pos[i].y - mouse.y) * (pos[i].y - mouse.y);
//        //    }
//
//        //    int selected = distance[0] < distance[1] ? 0 : 1;
//        //    if (distance[selected] < (4 * GRAB_RADIUS * 4 * GRAB_RADIUS))
//        //    {
//        //        SetTooltip("(%4.3f, %4.3f)", P[selected * 2 + 0], P[selected * 2 + 1]);
//
//        //        if (/*hovered &&*/ (IsMouseClicked(0) || IsMouseDragging(0))) {
//        //            float& px = (P[selected * 2 + 0] += GetIO().MouseDelta.x / Canvas.x);
//        //            float& py = (P[selected * 2 + 1] -= GetIO().MouseDelta.y / Canvas.y);
//
//        //            if (AREA_CONSTRAINED) {
//        //                px = (px < 0 ? 0 : (px > 1 ? 1 : px));
//        //                py = (py < 0 ? 0 : (py > 1 ? 1 : py));
//        //            }
//
//        //            changed = true;
//        //        }
//        //    }
//        //}
//
//        ////if (hovered || changed) DrawList->PushClipRectFullScreen();
//
//        //// draw curve
//        //{
//        //    ImColor color(GetStyle().Colors[ImGuiCol_PlotLines]);
//        //    for (int i = 0; i < SMOOTHNESS; ++i) {
//        //        ImVec2 p = { results[i + 0].x, 1 - results[i + 0].y };
//        //        ImVec2 q = { results[i + 1].x, 1 - results[i + 1].y };
//        //        ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
//        //        ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
//        //        DrawList->AddLine(r, s, color, CURVE_WIDTH);
//        //    }
//        //}
//
//        //// draw lines and grabbers
//        //float luma = IsItemActive() || IsItemHovered() ? 0.5f : 1.0f;
//        //ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
//        //ImVec2 p1 = ImVec2(P[0], 1 - P[1]) * (bb.Max - bb.Min) + bb.Min;
//        //ImVec2 p2 = ImVec2(P[2], 1 - P[3]) * (bb.Max - bb.Min) + bb.Min;
//        //DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), p1, ImColor(white), LINE_WIDTH);
//        //DrawList->AddLine(ImVec2(bb.Max.x, bb.Min.y), p2, ImColor(white), LINE_WIDTH);
//        //DrawList->AddCircleFilled(p1, GRAB_RADIUS, ImColor(white));
//        //DrawList->AddCircleFilled(p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
//        //DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(white));
//        //DrawList->AddCircleFilled(p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));
//
//        return true;
//    }
//
//    void ShowBezierDemo() {
//        { static float v[5] = { 0.950f, 0.050f, 0.795f, 0.035f }; Bezier("easeInExpo", v); }
//    }
//}

#pragma endregion

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
        30 // fontSize
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
    bitDepth(UIResult::TransferFunction::BitDepth::BIT_DEPTH_10_BIT),
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


void renderAppUI(UIResult& result, const UIResult& values)
{
#ifdef BRWL_USE_DEAR_IM_GUI
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
        End();
    }

    if (showTools)
    {
        thread_local float plotWidth = 1;
        thread_local float plotHeight = 1;
        thread_local float menuSpaceY = 0; // the height of all the stuff before the graph plot comes
        thread_local bool fitWindow = false;
        //{
        //    const float minWindowSizeX = result.transferFunction.getArrayLength() * plotWidth + 25;
        //    const float minWindowSizeY = Utils::min(GetIO().DisplaySize.y - 20, menuSpaceY + result.transferFunction.getArrayLength() * plotHeight + 20);
        //    if (fitWindow) {
        //        ImGui::SetNextWindowSize({ minWindowSizeX + 10, minWindowSizeY + 10 });
        //        fitWindow = false;
        //    }

        //    PushStyleVar(ImGuiStyleVar_WindowMinSize, { minWindowSizeX, minWindowSizeY });
        //}

        Begin("Tools", &showTools);
        {

            // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
            static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
            ImGui::CheckboxFlags("ImGuiTabBarFlags_Reorderable", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_Reorderable);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_AutoSelectNewTabs", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_AutoSelectNewTabs);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_TabListPopupButton", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_TabListPopupButton);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_NoCloseWithMiddleMouseButton", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
            if ((tab_bar_flags & ImGuiTabBarFlags_FittingPolicyMask_) == 0)
                tab_bar_flags |= ImGuiTabBarFlags_FittingPolicyDefault_;
            if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyResizeDown", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_FittingPolicyResizeDown))
                tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyResizeDown);
            if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyScroll", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_FittingPolicyScroll))
                tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyScroll);

            // Tab Bar
            const char* names[4] = { "Artichoke", "Beetroot", "Celery", "Daikon" };
            static bool opened[4] = { true, true, true, true }; // Persistent user state
            for (int n = 0; n < IM_ARRAYSIZE(opened); n++)
            {
                if (n > 0) { ImGui::SameLine(); }
                ImGui::Checkbox(names[n], &opened[n]);
            }

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
            // the underlying bool will be set to false when the tab is closed.
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                for (int n = 0; n < IM_ARRAYSIZE(opened); n++)
                    if (opened[n] && ImGui::BeginTabItem(names[n], &opened[n], ImGuiTabItemFlags_None))
                    {
                        ImGui::Text("This is the %s tab!", names[n]);
                        if (n & 1)
                            ImGui::Text("I am an odd tab.");
                        ImGui::EndTabItem();
                    }
                ImGui::EndTabBar();
            }

           // ENUM_SELECT("Bit Depth", values.transferFunction.bitDepth, result.transferFunction.bitDepth, UIResult::TransferFunction, BitDepth, bitDepthNames);

           // thread_local bool lockAspect = false;
           // float plotWidthBefore = plotWidth;
           // float plotHeihgtBefore = plotHeight;
           // // Draw first plot with multiple sources
           // Text("Plot Size: ");
           // SLIDER_FIX(1);
           // Text("Width: "); SameLine(); SliderFloat("", &plotWidth, 1, 3, "");
           // if (!lockAspect) { SameLine(); if (Button("Reset")) plotWidth = 1; }
           // SLIDER_FIX_END();
           // SLIDER_FIX(2);
           // Text("Height: "); SameLine(); SliderFloat("", &plotHeight, 1, 3, "");
           // if (!lockAspect) { SameLine(); if (Button("Reset")) plotHeight = 1; }
           // SLIDER_FIX_END();
           // if (lockAspect) {
           //     if (plotWidth != plotWidthBefore) plotHeight = plotWidth;
           //     else if (plotHeight != plotHeihgtBefore) plotWidth = plotHeight;
           //     else if (plotHeight != plotWidth) plotHeight = plotWidth = Utils::max(plotHeight, plotWidth);
           // }

           // Checkbox(" Maintain Aspect Ratio", &lockAspect);
           // if (lockAspect) { SameLine(); if (Button("Reset")) plotWidth = plotHeight = 1; }
           // thread_local bool showCoordinatesTooltip = true;
           // Checkbox(" Show Tooltip in Graph", &showCoordinatesTooltip); 
           // thread_local float ctrlPtointScale = 1;
           // SLIDER_FIX(3);
           // Text("Control Point Size: "), SameLine(); SliderFloat("", &ctrlPtointScale, 0.2f, 3.f);
           // SLIDER_FIX_END();
           //fitWindow = Button(" Fit Window");

           // menuSpaceY = ImGui::GetCursorPosY();

           // bool ctrlPointsChanged = false;
           // ImGui::PlotConfig conf{ };
           // conf.values.count = result.transferFunction.getArrayLength();
           // conf.values.ys = result.transferFunction.transferFunction; // use ys_list to draw several lines simultaneously
           // conf.values.ys_count = 1;
           // const ImU32 graphColor(ImColor(0, 255, 0));
           // conf.values.colors = &graphColor;
           // conf.scale.min = 0;
           // conf.scale.max = 1;
           // conf.tooltip.show = showCoordinatesTooltip;
           // conf.tooltip.format = "Idx: %g, Cursor Value: %8.4g, Graph Value %8.4g";
           // conf.grid_x.show = true;
           // conf.grid_x.size = 128;
           // conf.grid_x.subticks = 4;
           // conf.grid_y.show = true;
           // conf.grid_y.size = 1.0f;
           // conf.grid_y.subticks = 5;
           // conf.selection.show = false;
           // conf.ctrlPoints = &values.transferFunction.controlPoints;
           // conf.ctrlPointSize = ctrlPtointScale;
           // conf.values.ctrlPoints = &result.transferFunction.controlPoints;
           // conf.values.ctrlPointsChanged = &ctrlPointsChanged;
           // //conf.selection.start = &selection_start;
           // //conf.selection.length = &selection_length;
           // int texSideLength = values.transferFunction.getArrayLength();
           // float width = texSideLength * plotWidth;
           // float height = texSideLength * plotHeight;
           // conf.frame_size = ImVec2(width, height);
           // if (values.transferFunction.textureID != nullptr)
           // {
           //     conf.useBackGroundTextrue = true;
           //     conf.texID = values.transferFunction.textureID;
           //     conf.maxTexVal = (float)texSideLength;
           // }

           // ImGui::Plot("plot1", conf);
           // ctrlPointsChanged |= values.transferFunction.bitDepth != result.transferFunction.bitDepth;
           // if (ctrlPointsChanged)
           // {
           //     decltype(result.transferFunction.controlPoints)& array = result.transferFunction.controlPoints;
           //     std::sort(array.begin(), array.end(), [](const Vec2& a, const Vec2& b) {return a.x < b.x; });
           //     if (array.size() >= 2)
           //     {
           //         // remove duplicates
           //         array.erase(std::remove_if(array.begin() + 1, array.end(), [&array](::BRWL::Vec2& v){return v.x == array[&v - array.data() - 1].x; }), array.end());
           //     }
           //     result.transferFunction.updateFunction();
           // }

        }
        End();
        PopStyleVar();
    }

#endif // BRWL_USE_DEAR_IM_GUI

}


BRWL_NS_END
