#include "ImGui/imgui_plot.h"
#include "ImGui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImGui/imgui_internal.h"

namespace ImGui {


static void cursor_to_normalized_coords(const ImVec2& pos, ImVec2& normalizedOut, const ImRect& bb);

static void cursor_to_idx(const ImVec2& pos, int& idx, float& amplitude, const ImRect& bb, int count)
{
    ImVec2 out;
    cursor_to_normalized_coords(pos, out, bb);
    idx = (int)(out.x * (count - 1));
    amplitude = out.y;
    IM_ASSERT(idx < count);
}

static void cursor_to_normalized_coords(const ImVec2& pos, ImVec2& normalizedOut, const ImRect& bb)
{
    normalizedOut.x = ImClamp((pos.x - bb.Min.x) / (bb.Max.x - bb.Min.x), 0.0f, 0.9999f);
    normalizedOut.y = 1 - ImClamp((pos.y - bb.Min.y) / (bb.Max.y - bb.Min.y), 0.0f, 0.9999f);
}

PlotStatus Plot(const char* label, const PlotConfig& conf, const bool forceDisplayHandles)
{
    PlotStatus status = PlotStatus::nothing;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return status;

    const float* const* ys_list = conf.values.ys_list;
    int ys_count = conf.values.ys_count;
    const ImU32* colors = conf.values.colors;
    if (conf.values.ys != nullptr) { // draw only a single plot
        ys_list = &conf.values.ys;
        ys_count = 1;
        colors = &conf.values.color;
    }

    const ImGuiStyle& style = GetStyle();
    const ImGuiID id = window->GetID(label);

    const ImRect inner_bb(
        window->DC.CursorPos + style.FramePadding,
        window->DC.CursorPos + style.FramePadding + conf.frame_size);
    const ImRect frame_bb(
        inner_bb.Min - style.FramePadding,
        inner_bb.Max + style.FramePadding);
    const ImRect total_bb = frame_bb;
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0, &frame_bb))
        return status;
    const bool hovered = ItemHoverable(frame_bb, id);

    RenderFrame(
        frame_bb.Min,
        frame_bb.Max,
        GetColorU32(ImGuiCol_FrameBg),
        true,
        style.FrameRounding);

    if (conf.useBackGroundTextrue)
    {
        ImVec2 t0(0, 0);
        ImVec2 t1(1, 1);
        float maxTexVal = conf.maxTexVal ? conf.maxTexVal : 1;  // protect div by 0
        window->DrawList->AddImage(conf.texID, inner_bb.Min, inner_bb.Max + ImVec2(1, 1), t0, t1, static_cast<ImU32>(ImColor(1.0f / maxTexVal, 1.0f / maxTexVal, 1.0f / maxTexVal)));
    }

    if (conf.values.count > 0)
    {
        ImGuiIO& io = GetIO();
        int mouseXIdx = 0;
        float mouseY = 0;
        cursor_to_idx(io.MousePos, mouseXIdx, mouseY, inner_bb, conf.values.count);
        const bool mouseLeftClicked = io.MouseClicked[0];
        const bool mouseRightClicked = io.MouseClicked[1];
        const bool mouseLeftDown = io.MouseDown[0];

        int res_w;
        if (conf.skip_small_lines)
            res_w = ImMin((int)conf.frame_size.x, conf.values.count);
        else
            res_w = conf.values.count;
        res_w -= 1;
        int item_count = conf.values.count - 1;

        float x_min = (float)conf.values.offset;
        float x_max = (float)conf.values.offset + conf.values.count - 1;
        if (conf.values.xs) {
            x_min = conf.values.xs[size_t(x_min)];
            x_max = conf.values.xs[size_t(x_max)];
        }

        // Tooltip on hover
        int v_hovered = -1;
        if (conf.tooltip.show && hovered && inner_bb.Contains(io.MousePos)) {
            const size_t data_idx = conf.values.offset + (mouseXIdx % conf.values.count);
            const float x0 = conf.values.xs ? conf.values.xs[data_idx] : mouseXIdx;
            const float graphValue = ys_list[0][data_idx];
            SetTooltip(conf.tooltip.format, x0, mouseY, graphValue);
            v_hovered = mouseXIdx;
        }

        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (conf.scale.min == conf.scale.max) ?
            0.0f : (1.0f / (conf.scale.max - conf.scale.min));

        if (conf.grid_x.show) {
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
        }

        const ImU32 col_hovered = GetColorU32(ImGuiCol_PlotLinesHovered);
        ImU32 col_base = GetColorU32(ImGuiCol_PlotLines);

        for (int i = 0; i < ys_count; ++i) {
            if (colors) {
                if (colors[i]) col_base = colors[i];
                else col_base = GetColorU32(ImGuiCol_PlotLines);
            }
            float v0 = ys_list[i][conf.values.offset];
            float t0 = 0.0f;
            // Point in the normalized space of our target rectangle
            ImVec2 tp0 = ImVec2(t0, 1.0f - ImSaturate((v0 - conf.scale.min) * inv_scale));

            for (int n = 0; n < res_w; n++)
            {
                const float t1 = t0 + t_step;
                const int v1_idx = (int)(t0 * item_count + 0.5f);
                IM_ASSERT(v1_idx >= 0 && v1_idx < conf.values.count);
                const float v1 = ys_list[i][conf.values.offset + (v1_idx + 1) % conf.values.count];
                const ImVec2 tp1 = ImVec2(t1, 1.0f - ImSaturate((v1 - conf.scale.min) * inv_scale));

                // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
                ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
                ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, tp1);

                if (v1_idx == v_hovered) {
                    window->DrawList->AddCircleFilled(pos0, 3, col_hovered);
                }

                window->DrawList->AddLine(
                    pos0,
                    pos1,
                    col_base,
                    conf.line_thickness);

                t0 = t1;
                tp0 = tp1;
            }
        }

        if (conf.v_lines.show) {
            for (size_t i = 0; i < conf.v_lines.count; ++i)
            {
                const float t1 = conf.v_lines.indices[i] * t_step;
                ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(t1, 0.f));
                ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(t1, 1.f));
                window->DrawList->AddLine(pos0, pos1, IM_COL32(0xff, 0, 0, 0x88));
            }
        }

        *conf.values.ctrlPointsChanged = conf.values.ctrlPoints->update(window, hovered, inner_bb, GetIO().MousePos, conf.ctrlPointSize, forceDisplayHandles);

        if (conf.selection.show) {
            if (hovered) {
                if (mouseLeftClicked) {
                    SetActiveID(id, window);
                    FocusWindow(window);
                    int start = conf.values.offset + (mouseXIdx % conf.values.count);
                    int end = start;
                    if (conf.selection.sanitize_fn)
                        end = conf.selection.sanitize_fn(end - start) + start;
                    if (end < conf.values.offset + conf.values.count) {
                        *conf.selection.start = start;
                        *conf.selection.length = end - start;
                        status = PlotStatus::selection_updated;
                    }
                }
            }

            if (GetActiveID() == id)
            {
                if (mouseLeftDown)
                {
                    const int start = *conf.selection.start;
                    int end = conf.values.offset + (mouseXIdx % conf.values.count);
                    if (end > start) {
                        if (conf.selection.sanitize_fn)
                            end = conf.selection.sanitize_fn(end - start) + start;
                        if (end < conf.values.offset + conf.values.count) {
                            *conf.selection.length = end - start;
                            status = PlotStatus::selection_updated;
                        }
                    }
                }
                else {
                    ClearActiveID();
                }
            }

            float fSelectionStep = 1.0f / item_count;
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max,
                ImVec2(fSelectionStep * *conf.selection.start, 0.f));
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max,
                ImVec2(fSelectionStep * (*conf.selection.start + *conf.selection.length), 1.f));
            window->DrawList->AddRectFilled(pos0, pos1, IM_COL32(128, 128, 128, 32));
            window->DrawList->AddRect(pos0, pos1, IM_COL32(128, 128, 128, 128));
        }




    }
    
    // Text overlay
    if (conf.overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, conf.overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));

    return status;
}
}