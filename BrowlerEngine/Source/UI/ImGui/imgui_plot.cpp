#include "ImGui/imgui_plot.h"
#include "ImGui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImGui/imgui_internal.h"

namespace ImGui
{


static void cursor_to_normalized_coords(const ImVec2& pos, ImVec2& normalizedOut, const ImRect& bb)
{
    normalizedOut.x = ImClamp((pos.x - bb.Min.x) / (bb.Max.x - bb.Min.x), 0.0f, 0.9999f);
    normalizedOut.y = 1 - ImClamp((pos.y - bb.Min.y) / (bb.Max.y - bb.Min.y), 0.0f, 0.9999f);
}

void Plot(const char* label, const PlotConfig& conf, const bool forceDisplayHandles)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return;

    ImRect inner_bb;
    bool culled, hovered;
    const ImGuiID id = DrawFrame(window, label, {(float)conf.frameSizeX, (float)conf.frameSizeY}, true, &inner_bb, &culled, &hovered);
    
    if (culled || conf.frameSizeX == 0 || conf.frameSizeY == 0) return;

    if (conf.useBackGroundTextrue)
    {
        ImVec2 t0(0, 0);
        ImVec2 t1(1, 1);
        float maxTexVal = conf.maxTexVal ? conf.maxTexVal : 1;  // protect div by 0
        window->DrawList->AddImage(conf.texID, inner_bb.Min, inner_bb.Max + ImVec2(1, 1), t0, t1, static_cast<ImU32>(ImColor(1.0f / maxTexVal, 1.0f / maxTexVal, 1.0f / maxTexVal)));
    }

    ImGuiIO& io = GetIO();
    ImVec2 normalizedMouse;
    cursor_to_normalized_coords(io.MousePos, normalizedMouse, inner_bb);
    const bool isHovered = conf.tooltip.show && hovered && inner_bb.Contains(io.MousePos);
    const bool skipSmall = conf.skipSmallLines;
    // Todo: gridlines
    /* if (conf.grid_x.show) {
        float y0 = inner_bb.Min.y;
        float y1 = inner_bb.Max.y;
        float cnt = conf.values.count / (conf.grid_x.size / conf.grid_x.subticks);
        float inc = 1.f / cnt;
        for (int i = 0; i <= cnt; ++i) {
            float x0 = ImLerp(inner_bb.Min.x, inner_bb.Max.x, i * inc);
            window->DrawList->AddLine(
                ImVec2(x0, y0),
                ImVec2(x0, y1),
                IM_COL32(200, 200, 200, (i % conf.grid_x.subticks) ? 128 : 255));
        }
    }
    if (conf.grid_y.show) {
        float x0 = inner_bb.Min.x;
        float x1 = inner_bb.Max.x;
        float cnt = (conf.scale.max - conf.scale.min) / (conf.grid_y.size / conf.grid_y.subticks);
        float inc = 1.f / cnt;
        for (int i = 0; i <= cnt; ++i) {
            float y0 = ImLerp(inner_bb.Min.y, inner_bb.Max.y, i * inc);
            window->DrawList->AddLine(
                ImVec2(x0, y0),
                ImVec2(x1, y0),
                IM_COL32(0, 0, 0, (i % conf.grid_y.subticks) ? 16 : 64));
        }
    }*/

    const ImU32 col_hovered = GetColorU32(ImGuiCol_PlotLinesHovered);

    for (unsigned int i = 0; i < conf.values.nPlots; ++i)
    {
        const unsigned int n = conf.values.nSamples[i];
        
        if (!n) continue;
        
        const ImU32 color = conf.values.colors ? conf.values.colors[i] : GetColorU32(ImGuiCol_PlotLines);
        const float* yValues = conf.values.yValues[i] + conf.values.yBaseIndices[i];
        const unsigned int yStride = conf.values.yStrides[i];
        const float* xValues = conf.values.xValues[i] + (conf.values.xValues[i] ? conf.values.xBaseIndices[i] : 0);
        const unsigned int xStride = xValues ? conf.values.xStrides[i] : 1;
        CtrlPointGroup* ctrlPointGroup = conf.values.ctrlPoints[i];
        bool& ctrlPointsChanged  = conf.values.ctrlPointsChanged[i];
        ctrlPointsChanged = false;
        //unsigned int res_w = (conf.skip_small_lines ? ImMin(conf.frameSizeX, n) : n) - 1;



        //const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (conf.scale.min == conf.scale.max) ? 0.0f : (1.0f / (conf.scale.max - conf.scale.min));

        const float* y = yValues;
        const float* x = xValues;

        float xPos = x ? *x : 0; // if x is nullptr, then use index
        ImVec2 pt0 = ImVec2(xPos, 1.0f - ImSaturate((*y - conf.scale.min) * inv_scale)); // normalized coords
        ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, pt0);
        const int item_count = n - 1;

        for (unsigned int j = 1; j < n; ++j)
        {
            xPos = x ? *x : (float)j / (float)n; // if x is nullptr, then use index
            const ImVec2 pt1 = ImVec2(xPos, 1.0f - ImSaturate((*y - conf.scale.min) * inv_scale)); // normalized coords
            const ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, pt1);

            // todo: maybe into if below
            if (isHovered && pt0.x < normalizedMouse.x && pt1.x >= normalizedMouse.x)
            {
                // todo: evtl add lerp
                window->DrawList->AddCircleFilled(pos0, 3, col_hovered);
            }

            if (std::abs(pos1.x - pos0.x) >= 1.0)
            {
                window->DrawList->AddLine(pos0, pos1, color, conf.line_thickness);
                pt0 = pt1;
                pos0 = pos1;
            }

            y += yStride;
            if (x) {
                x += xStride;
            }
        }

        if (conf.values.editPlotIdx == i)
        {
            ctrlPointsChanged = ctrlPointGroup->update(window, hovered, inner_bb, GetIO().MousePos, conf.ctrlPointSize, forceDisplayHandles);
        }
    }

    
    //// Text overlay
    //if (conf.overlay_text)
    //    RenderTextClipped(ImVec2(inner_bb.Min.x, inner_bb.Min.y ), inner_bb.Max, conf.overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));
}


}