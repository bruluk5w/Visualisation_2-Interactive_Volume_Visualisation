#pragma once // (c) 2020 Lukas Brunner
#include "UI/ImGui/imgui.h"

#include "UI/ImGui/ImGuiHelpers.h"

namespace ImGui
{


    // Use this structure to pass the plot data and settings into the Plot function
    struct PlotConfig {
        struct Values {
            // the number of plots
            unsigned int nPlots = 0;
            // the number of samples in each plot
            const unsigned int* nSamples = nullptr;

            // index of first sample of respective plot in respective float array in yValues
            const unsigned int* yBaseIndices = nullptr;
            // number of pointer increments between consecutive float samples in the respective array in yValues
            const unsigned int* yStrides = nullptr;
            // array of pointers to y values
            const float* const* yValues = nullptr;

            // index of first sample position of respective plot in respective float array in xValues
            const unsigned int* xBaseIndices = nullptr;
            // number of pointer increments between consecutive floats in the respective array in xValues
            const unsigned int* xStrides = nullptr;
            // array of pointers to x positions
            const float* const* xValues = nullptr;

            // colors for each plot (optional)
            const ImU32* colors = nullptr;

            // index of the plot to edit
            unsigned int editPlotIdx = 0;

            // control points for each plot
            CtrlPointGroup* const* ctrlPoints = nullptr;

            // dirty flags for each plot
            bool* ctrlPointsChanged = nullptr;
        } values;
        struct Scale {
            // Minimum plot value
            float min;
            // Maximum plot value
            float max;
        } scale;
        struct Tooltip {
            bool show = false;
            const char* format = "%g / %g: %8.4g";
        } tooltip;
        //struct Grid {
        //    bool show = false;
        //    float size = 100; // at which intervals to draw the grid
        //    int subticks = 10; // how many subticks in each tick
        //} grid_x, grid_y;

        unsigned int frameSizeX = 0;
        unsigned int frameSizeY = 0;
        float line_thickness = 1.f;
        bool skipSmallLines = true;
        //const char* overlay_text = nullptr;

        bool useBackGroundTextrue = false;
        ImTextureID texID = nullptr;
        float maxTexVal = 1;
        float ctrlPointSize = 1;
    };

    IMGUI_API void Plot(const char* label, const PlotConfig& conf, const bool forceHovered=false);


}
