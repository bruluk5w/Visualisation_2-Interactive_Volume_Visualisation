#include "AppUi.h"

#include "UI/ImGui/imgui.h"
#include "UI/ImGui/imgui_plot.h"

#include <fstream>
#include <filesystem>


BRWL_NS


const char* UIResult::Settings::fontNames[] = {
    "Open Sans Light",
    "Open Sans Regular",
    "Open Sans Semibold",
    "Open Sans Bold"
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
        22, // fontSize
        1.0f, // numSlicesPerVoxel
        false, // vsync
        false, // freeCamMovement
        false, // drawAssetBoundaries
        false, // drawViewingVolumeBoundaries
        false, // drawOrthographicXRay
    },
    transferFunctions(),
    light{
        {0.f, 0.f, 1.f}, // coords
        {1.8f, 1.8f, 1.8f, 1.f} // color
    }
{ }


UIResult::TransferFunctionCollection::TransferFunctionCollection() :
    refractionTransFunc({
        {   // no refraction
            { {{0, 0.5f}}, {{1, 0.5f}} }
        }
    }),
    particleColorTransFunc({
        {   // white particle color
            { {{0, 0.0f}}, {{1, 0.0f}} },
            { {{0, 0.0f}}, {{1, 0.0f}} },
            { {{0, 0.0f}}, {{1, 0.0f}} },
        }
    }),
    opacityTransFunc(), 
    mediumColorTransFunc({
        {   // clear medium
            { {{0, 1.0f}}, {{1, 1.0f}} },
            { {{0, 1.0f}}, {{1, 1.0f}} },
            { {{0, 1.0f}}, {{1, 1.0f}} },
        }
    }),
    array { 
        &refractionTransFunc,
        &particleColorTransFunc,
        &opacityTransFunc,
        &mediumColorTransFunc,
    }
{ }

namespace {
    static const char* transferFunctionSavePath = "./Assets/tf";
    static const char* transferFunctionSavePathTemplate = "./Assets/tf/%s";
    
    static std::vector<std::string> tfFiles(0);

    void refreshTfFiles() {
        tfFiles.clear();
        std::string path = "/path/to/directory";
        char buf[512];
        for (const auto& entry : std::filesystem::directory_iterator(transferFunctionSavePath))
        {
            //ImTextStrToUtf8(buf, sizeof(buf), entry.path().string().c_str(), entry.path().string().end())
            tfFiles.emplace_back(entry.path().string());
        }
    }
}



void setupAppUI()
{
    BRWL_EXCEPTION(std::filesystem::is_directory("./Assets"), BRWL_CHAR_LITERAL("Asset directory not found."));
    if (!std::filesystem::is_directory(transferFunctionSavePath)) {
        bool created = std::filesystem::create_directory(transferFunctionSavePath);
        BRWL_EXCEPTION(created, BRWL_CHAR_LITERAL("Failed to create asset subfolder for transfer functions"));
    }

    refreshTfFiles();
}

using namespace ImGui;

#define ENUM_SELECT(label, current_val, result_val, scope, enum_name, name_array) \
{ \
    int item_current = ENUM_CLASS_TO_NUM(current_val); \
    ::ImGui::Text(label); \
    ::ImGui::Combo("", &item_current, scope::name_array, IM_ARRAYSIZE(scope::name_array)); \
    result_val = (scope::enum_name)Utils::clamp<std::underlying_type_t<scope::enum_name>>(item_current, ENUM_CLASS_TO_NUM(scope::enum_name::MIN), ENUM_CLASS_TO_NUM(scope::enum_name::MAX)); \
}

#define SLIDER_FIX(id, numLines) ImGui::BeginChild(#id, ImVec2(ImGui::GetWindowWidth(), ImGui::GetTextLineHeightWithSpacing() * numLines), false);
#define SLIDER_FIX_END() EndChild();


void drawHints(const UIResult& values);

void renderAppUI(UIResult& result, const UIResult& values)
{
#ifdef BRWL_USE_DEAR_IM_GUI

    drawHints(values);

    const ImGuiIO& io = GetIO();

    ImGui::BeginMainMenuBar();
    Text("%.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y); SameLine();
    Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    Dummy(ImVec2(40, 0));
    thread_local bool showSettings = false;
    thread_local bool showLightSettings = false;
    thread_local bool showTransferFunctions = false;
    showTransferFunctions ^= Button("Transfer Functions");
    showSettings ^= Button("Settings");
    showLightSettings ^= Button("Light");
    Text("Remaining Slices: %u", values.remainingSlices);
    ImGui::EndMainMenuBar();

    if (showSettings)
    {
        Begin("Settings", &showSettings);
        // work around for slider bug where slider is activated when clicking on the combo box above
        SLIDER_FIX(20, 2)
            ENUM_SELECT("GUI Font: ", values.settings.font, result.settings.font, UIResult::Settings, Font, fontNames);
        SLIDER_FIX_END()
        SLIDER_FIX(30, 2)
            Text("Font Size:");
            SliderFloat("", &result.settings.fontSize, 5, 40);
            result.settings.fontSize = Utils::clamp(result.settings.fontSize, 5.f, 40.f);
        SLIDER_FIX_END();

        SLIDER_FIX(40, 2)
            Text("Number of slices per voxel:");
            ::ImGui::InputFloat("", &result.settings.numSlicesPerVoxel, 0.01f, 2.f, "%0.2f");
            SliderFloat("", &result.settings.numSlicesPerVoxel, 0.01f, 2.f, "%0.2f");
            result.settings.numSlicesPerVoxel = Utils::clamp(result.settings.numSlicesPerVoxel, 0.01f, 2.f);
        SLIDER_FIX_END();
        Checkbox("VSync", &result.settings.vsync);
        Checkbox("Free Camera Movement", &result.settings.freeCamMovement);
        Checkbox("Draw Dataset Boundaries", &result.settings.drawAssetBoundaries);
        Checkbox("Draw Viewing Volume Boundaries", &result.settings.drawViewingVolumeBoundaries);
        Checkbox("Draw Orthographic X Ray (uses opacity transer function)", &result.settings.drawOrthographicXRay);
        End();
    }

    if (showTransferFunctions)
    {
        thread_local float plotWidth = 1;
        thread_local float plotHeight = 1;
        thread_local float menuSpaceY = 0; // the height of all the stuff before the graph plot comes
        thread_local bool fitWindow = false;

        {
            int arrayLen = result.transferFunctions.mediumColorTransFunc.getArrayLength();
            const float minWindowSizeX = arrayLen * plotWidth + 25.f;
            const float minWindowSizeY = Utils::min(GetIO().DisplaySize.y - 20, menuSpaceY + 20);
            if (fitWindow) {
                ImGui::SetNextWindowSize({ minWindowSizeX + 10, minWindowSizeY + 10 });
                fitWindow = false;
            }

            PushStyleVar(ImGuiStyleVar_WindowMinSize, { minWindowSizeX, minWindowSizeY });
        }

        Begin("Tools", &showTransferFunctions);
        {
           BaseTransferFunction::BitDepth resultBitDepth;
           ENUM_SELECT("Bit Depth", values.transferFunctions.array[0]->bitDepth, resultBitDepth, BaseTransferFunction, BitDepth, bitDepthNames);
           for (int i = 0; i < countof(result.transferFunctions.array); ++i)
           {
               result.transferFunctions.array[i]->bitDepth = resultBitDepth;
           }

            thread_local bool lockAspect = false;
            float plotWidthBefore = plotWidth;
            float plotHeihgtBefore = plotHeight;
            // Draw first plot with multiple sources
            Text("Plot Size: ");
            SLIDER_FIX(2, 2);
                Text("Width: ");
                SliderFloat("", &plotWidth, 1, 3, "");
                if (!lockAspect) { SameLine(); if (Button("Reset")) plotWidth = 1; }
            SLIDER_FIX_END();
            SLIDER_FIX(3, 2);
                Text("Height: ");
                SliderFloat("", &plotHeight, 1, 3, "");
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
            thread_local float ctrlPtointScale = 2.2f;
            SLIDER_FIX(4, 2);
                Text("Control Point Size: ");
                SliderFloat("", &ctrlPtointScale, 0.2f, 3.f);
                bool forcePlotHovered = ImGui::IsItemHovered();
            SLIDER_FIX_END();

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
            // the underlying bool will be set to false when the tab is closed.
            if (ImGui::BeginTabBar("Transfer Functions",
                ImGuiTabBarFlags_FittingPolicyResizeDown |
                ImGuiTabBarFlags_Reorderable | 
                ImGuiTabBarFlags_AutoSelectNewTabs
            ))
            {
                for (int i = 0; i < ENUM_CLASS_TO_NUM(UIResult::TransferFuncType::MAX); i++)
                {
                    // initially autoselect opacity tab when opening for the first time
                    thread_local bool openedDefault = false;
                    ImGuiTabItemFlags flags = openedDefault || i != ENUM_CLASS_TO_NUM(UIResult::TransferFuncType::OPACITY) ? ImGuiTabItemFlags_None : ImGuiTabItemFlags_SetSelected;
                    if (ImGui::BeginTabItem(UIResult::TransferFunctionCollection::transferFuncNames[i], nullptr, flags))
                    {
                        if (flags == ImGuiTabItemFlags_SetSelected)
                            openedDefault = true;

                        Text(UIResult::TransferFunctionCollection::transferFuncNames[i]);
                        const BaseTransferFunction* valueTFunc = values.transferFunctions.array[i];
                        BaseTransferFunction* resultTFunc = result.transferFunctions.array[i];

                        const uint8_t numChannels = ImMin((uint8_t)4, resultTFunc->getNumChannels());
                        const unsigned int l = resultTFunc->getArrayLength();
                        const unsigned int nSamples[4] = { l, l, l, l };

                        // y layout
                        unsigned int yBaseIndices[4] = { 0 };
                        for (int j = 0; j < numChannels; ++j) {
                            yBaseIndices[j] = resultTFunc->getChannelOffsets(j);
                        }

                        unsigned int yStrides[4] = { 0 };
                        for (int j = 0; j < numChannels; ++j) {
                            yStrides[j] = resultTFunc->getChannelStrides(j);
                        }

                        const float* yData[4] = { nullptr };
                        {
                            const float* data = resultTFunc->getData();
                            for (int j = 0; j < numChannels; ++j) {
                                yData[j] = data;
                            }
                        }

                        // x layout
                        const float* xData[4] = { nullptr }; // use monotonic indices

                        // ctrl point groups
                        CtrlPointGroup* ctrlPoints[4]{ nullptr };
                        for (int j = 0; j < numChannels; ++j) {
                            ctrlPoints[j] = resultTFunc->getCtrlPointGroup(j);
                        }

                        bool ctrlPointsChanged[4] = { false };

                        thread_local int editChannelIdx = 0;
                        editChannelIdx = ImMin(editChannelIdx, (int)resultTFunc->getNumChannels() - 1);

                        ImGui::PlotConfig conf{ };
                        conf.values.nPlots = numChannels;
                        conf.values.nSamples = nSamples;
                        conf.values.yBaseIndices = yBaseIndices;
                        conf.values.yValues = yData;
                        conf.values.yStrides = yStrides;
                        conf.values.xValues = xData;

                        static const ImU32 rgbCol[3] { ImColor(255, 30, 30), ImColor(30, 255, 30), ImColor(30, 30, 255) };
                        static const ImU32 greyCol[1] { ImColor(255, 255, 255) };
                        const ImU32* colors = resultTFunc->getNumChannels() == 1 ? greyCol : rgbCol;
                        
                        conf.values.colors = colors;
                        conf.scale.min = 0;
                        conf.scale.max = 1;
                        conf.tooltip.show = showCoordinatesTooltip;
                        conf.tooltip.format = "Idx: %g, Cursor Value: %8.4g, Graph Value %8.4g";
                        //conf.grid_x.show = true;
                        //conf.grid_x.size = 128;
                        //conf.grid_x.subticks = 4;
                        //conf.grid_y.show = true;
                        //conf.grid_y.size = 1.0f;
                        //conf.grid_y.subticks = 5;
                        //conf.selection.show = false;
                        conf.values.editPlotIdx = editChannelIdx;
                        conf.ctrlPointSize = ctrlPtointScale;
                        conf.values.ctrlPoints = ctrlPoints;
                        conf.values.ctrlPointsChanged = ctrlPointsChanged;
                        unsigned int texSideLength = valueTFunc->getArrayLength();
                        BRWL_CHECK(plotWidth >= 0, nullptr);
                        unsigned int width = (unsigned int )(texSideLength * plotWidth);
                        unsigned int height = (unsigned int)(texSideLength * plotHeight);
                        conf.frameSizeX = width;
                        conf.frameSizeY = height;
                        if (valueTFunc->textureID != nullptr)
                        {
                            conf.useBackGroundTextrue = true;
                            conf.texID = valueTFunc->textureID;
                            conf.maxTexVal = 1;
                        }
                        
                        ImGui::Plot("plot1", conf, forcePlotHovered);

                        for (int j = 0; j < numChannels; ++j) {
                            ctrlPointsChanged[j] |= valueTFunc->bitDepth != resultTFunc->bitDepth;
                        }

                        resultTFunc->updateFunction(ctrlPointsChanged);

                          
                        Text("Edit Channel");
                        const static char* rgbLabel[] = { "R", "G", "B" };
                        for (int channelIdx = 0; channelIdx < ImMin(countof(rgbLabel), (size_t)resultTFunc->getNumChannels()); ++channelIdx)
                        {
                             SameLine(); RadioButton(rgbLabel[channelIdx], &editChannelIdx, channelIdx);
                        }

                        ImGui::EndTabItem();
                    }
                }

                ImGui::EndTabBar();
            }
            // save transfer functions
            thread_local char* tfError = nullptr;
            thread_local char tfName[512];
            InputText("Save Name", tfName, IM_ARRAYSIZE(tfName));
            // remove non alphanumeric chars and allow underscore
            *std::remove_if(std::begin(tfName), std::end(tfName), [](const char c) { return !(std::isalnum(c) || c == '_'); }) = '\0';
            if (Button("Save"))
            {
                tfError = nullptr;
                char path[512];
                ImFormatString(path, IM_ARRAYSIZE(path), transferFunctionSavePathTemplate, tfName);
                std::ofstream output(path, std::ios::binary);
                if (output) {
                    for (int i = 0; i < ENUM_CLASS_TO_NUM(UIResult::TransferFuncType::MAX); i++)
                    {
                        result.transferFunctions.array[i]->save(&output);
                    }
                }
                else
                {
                    tfError = "Could not save transfer function to file.";
                }
            } SameLine();

            if (Button("Refresh Load List"))
            {
                refreshTfFiles();
            }

            thread_local int selectedTf = -1;
            selectedTf = ImMin(selectedTf, (int)tfFiles.size() - 1);
            SLIDER_FIX(5, 2)
            if (BeginCombo("", "Select Transfer Function"))
            {
                for (int i = 0; i < tfFiles.size(); ++i)
                {
                    PushID((void*)(intptr_t)i);
                    if (Selectable(tfFiles[i].c_str()))
                    {
                        tfError = nullptr;
                        selectedTf = i;
                        char path[512];
                        ImFormatString(path, IM_ARRAYSIZE(path), transferFunctionSavePathTemplate, tfName);
                        std::ifstream input(path, std::ios::binary);
                        if (input) {
                            for (int i = 0; i < ENUM_CLASS_TO_NUM(UIResult::TransferFuncType::MAX); i++)
                            {
                                result.transferFunctions.array[i]->load(&input);
                            }
                        }
                        else
                        {
                            tfError = "Could not load transfer function from file.";
                        }

                        PopID();
                        break;
                    }

                    PopID();


                }

                EndCombo();
            }
            SLIDER_FIX_END();

            if (tfError)
            {
                TextColored(ImVec4(1, 0, 0, 1), tfError);
            }

            // remember Y position for window fit
            menuSpaceY = ImGui::GetCursorPosY();
        }

        End(); // Tools
        PopStyleVar();
    }
    
    if (showLightSettings)
    {
        Begin("Light Settings", &showLightSettings);
        Text("Light direction (relative to camera):");
        SLIDER_FIX(6, 1)
            SliderFloat3("", &result.light.coords.x, -1, 1);
            result.light.coords.z = BRWL::Utils::max(result.light.coords.z, 0.1f);
        SLIDER_FIX_END()
        Text("Light Color:");
        float col[4]{ values.light.color.x, values.light.color.y, values.light.color.z };
        ColorEdit4("", col, ImGuiColorEditFlags_HDR);
        result.light.color = Vec4(col[0], col[1], col[2], Utils::max(0.f, col[3]));
        End();
    }

#endif // BRWL_USE_DEAR_IM_GUI

}

#ifdef BRWL_USE_DEAR_IM_GUI

void drawHints(const UIResult& values)
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
    if (!io.WantCaptureMouse)
    {

        if (false)
        {
            SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            Begin("HintWindowCenter", nullptr, dummyFlags);
            Text("Rotate FPS view: <SPACE> + <MOUSE LEFT>");
            End();
        }
        else
        {
            SetNextWindowPos(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always, ImVec2(1.f, 1.f));
            Begin("HintWindowBottom", nullptr, dummyFlags);
            if (values.settings.freeCamMovement)
            {
                Text("Strave: <WSAD> | Up: <E> | Down: <Q> | Rotate FPS view: <MOUSE LEFT>");
            }
            else
            {
                Text("Orbit View: <WSAD> or <LeftMouse> + <Drag> | Zoom: <Q/E>");
            }

            End();
        }
    }
}

#endif // BRWL_USE_DEAR_IM_GUI


BRWL_NS_END
