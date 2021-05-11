#pragma once // (c) 2020 Lukas Brunner
#include "ImGui/imgui.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImGui/imgui_internal.h"

namespace ImGui
{
	void DrawArrow(ImGuiWindow* window, const ImRect box, const ImVec2 normalizedShaft, const ImVec2 normalizedTip, float tipAngle=0.1f, float tipLength=0.01f, float lineStrength=1.0f);
    ImGuiID DrawFrame(ImGuiWindow* window, const char* label, const ImVec2 frameSize, bool border, ImRect* innerBoxOut=nullptr, bool* culled=nullptr, bool* hovered=nullptr);

	struct CtrlPoint {
        CtrlPoint(ImVec2 normalizedMousePos) :
            pt(normalizedMousePos.x, 1 - normalizedMousePos.y),
            offset(),
            selected(false)
        { }

        CtrlPoint() :
            pt(),
            offset(),
            selected(false)
        { }

        CtrlPoint& operator=(const CtrlPoint& o) {
            pt = o.pt;
            offset = o.offset;
            selected = o.selected;
            return *this;
        }

		//! <summary>
		/** Renders the control point and returns a hint that can be used to decide whether this control point should be selected
		 * </summary>
		 * \param window The window to draw the control point for
		 * \param box The box within the normalized coordinates of the control point refer to
		 * \param mousePos The normalized mouse position with respect to box
		 * \param ptBaseSize The size of the control point in its smallest state
		 * \returns The positive squared distance between mousePos and the control point if mousePos is near enough to also select the control point. A negative float else.
         */
        float updateHover(ImGuiWindow* window, const ImRect box, const ImVec2 mousePos, const float ptBaseSize) const;

        //! returns true if the control point position moved
        bool setSelected(const ImVec2& mousePos, bool selected);

        ::BRWL::Vec2 pt;
        ImVec2 offset;
        bool selected;
    };

	struct CtrlPointGroup
    {
        CtrlPointGroup() :
            selected(-1),
            points{{ {{0, 1}}, {{1, 0}} }},
            refs{0, 1}
        { }

        CtrlPointGroup& operator=(const CtrlPointGroup& o) {
            selected = o.selected;
            points = o.points;
            refs = o.refs;
            return *this;
        }


        //! Returns true if the control point positions have changed
        bool update(ImGuiWindow* window, const bool isHovering, const ImRect& box, const ImVec2& mousePos, const float ctrlPointScale, const bool forceDisplayHandles = false);

        void sortRefs();
        void removeDuplicates();
        size_t size() { return points.size(); }
        const ::BRWL::Vec2& at(size_t idx) { return points[refs[idx]].pt; }

        int selected;
        std::vector<CtrlPoint> points;
        std::vector<size_t> refs;

	};
}
