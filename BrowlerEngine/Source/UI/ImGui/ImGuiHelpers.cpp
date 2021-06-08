#include "ImGui/ImGuiHelpers.h"

#include <numeric>
#include <fstream>

namespace ImGui
{

	void DrawArrow(ImGuiWindow* window, const ImRect box, const ImVec2 normalizedShaft, const ImVec2 normalizedTip, float tipAngle, float tipLength, float lineStrength)
	{
		if (window->SkipItems)
			return;

		ImVec2 shaft = ImLerp(box.Min, box.Max, normalizedShaft);
		ImVec2 tip = ImLerp(box.Min, box.Max, normalizedTip);
		ImVec2 direction = tip - shaft;
		direction = direction * ImInvLength(direction, 0);

		ImVec2 tipRight = ImRotate(direction, ImCos(tipAngle), ImSin(tipAngle)) * tipLength;
		ImVec2 tipLeft = ImRotate(direction, ImCos(-tipAngle), ImCos(-tipAngle)) * tipLength;

		window->DrawList->AddLine(shaft, tip, IM_COL32_WHITE, lineStrength);
		window->DrawList->AddLine(tipRight, tip, IM_COL32_WHITE, lineStrength);
		window->DrawList->AddLine(tipLeft, tip, IM_COL32_WHITE, lineStrength);
	}

    ImGuiID DrawFrame(ImGuiWindow* window, const char* label, const ImVec2 frameSize, bool border, ImRect* innerBoxOut, bool* culled, bool* hovered)
    {
        const ImGuiStyle& style = GetStyle();
        const ImGuiID id = window->GetID(label);

        const ImRect innerBox(window->DC.CursorPos + style.FramePadding, window->DC.CursorPos + style.FramePadding + frameSize);
        const ImRect outerBox(innerBox.Min - style.FramePadding, innerBox.Max + style.FramePadding);
        if (innerBoxOut)
            *innerBoxOut = innerBox;

        ItemSize(outerBox, style.FramePadding.y);
        if (!ItemAdd(outerBox, 0, &outerBox)) {
            if (culled)
                *culled = true;

            return id;
        }

        if (culled)
            *culled = false;

        if (hovered)
            *hovered = ItemHoverable(outerBox, id);

        RenderFrame(outerBox.Min, outerBox.Max, GetColorU32(ImGuiCol_FrameBg), border, style.FrameRounding);

        return id;
    }


    float CtrlPoint::updateHover(ImGuiWindow* window, const ImRect box, const ImVec2 mousePos, const float ptBaseSize) const
    {
        const float minDim = ImMin(box.GetWidth(), box.GetHeight());
        ImVec2 v = ImVec2(pt.x, 1-pt.y);
        const float norm2 = (mousePos.x - v.x) * (mousePos.x - v.x) + (mousePos.y - v.y) * (mousePos.y - v.y);
        float ptSize = 0.02f * ptBaseSize;
        ImColor col = ImColor(1.f, 1.f, 1.f, 0.5f);
        bool wantSelect = false;
        if (selected)
        {
            col = ImColor(1.0f, 0.5f, 0.5f, 0.7f);
            wantSelect = true;
        }
        else if (norm2 < 4.0f * ptSize * ptSize)
        {
            ptSize *= 2.0f;
            col = ImColor(1.f, 1.f, 0.5f);
            wantSelect = true;
        }

        window->DrawList->AddCircleFilled(box.Min + v * (box.Max - box.Min), minDim * ptSize, col);
        return wantSelect ? norm2 : -1;
    }

    bool CtrlPoint::setSelected(const ImVec2& mousePos, bool selected)
    {
        ImVec2 v = ImVec2(pt.x, 1 - pt.y);
        if (this->selected != selected)
        {
            if (selected)
            {   // select point
                offset.x = v.x - mousePos.x;
                offset.y = v.y - mousePos.y;
                this->selected = true;
            }
            else
            {   // deselect point
                offset = { };
                this->selected = false;
            }
        }
        else if (selected)
        {   // move point
            decltype(pt) ptOld = pt;
            pt.x = mousePos.x + offset.x;
            pt.y = 1 - (mousePos.y + offset.y);
            pt.x = (pt.x < 0 ? 0 : (pt.x > 1 ? 1 : pt.x));
            pt.y = (pt.y < 0 ? 0 : (pt.y > 1 ? 1 : pt.y));
            return pt != ptOld;
        }

        return false;
    }

    CtrlPointGroup::CtrlPointGroup(std::initializer_list<CtrlPoint> l) :
        selected(-1),
        points(l),
        refs(l.size())
    {
        BRWL_CHECK(l.size() >= 2 || l.size() == 0, nullptr);

        if (points.size() == 0) {
            points.emplace_back(ImVec2{ 0, 1 });
            points.emplace_back(ImVec2{ 1, 0 });
            refs.resize(2);
        }

        std::iota(refs.begin(), refs.end(), 0);
    }

    void CtrlPointGroup::save(std::ofstream* output)
    {
        sortRefs();
        const size_t size = refs.size();
        output->write((const char*)&size, sizeof(size));
        for (size_t i = 0; i < refs.size(); ++i)
        {
            const size_t ref = refs[i];
            const CtrlPoint& pt = points[ref];
            output->write((const char *)&pt.pt, sizeof(pt.pt));
        }
    }

    void CtrlPointGroup::load(std::ifstream* input)
    {
        size_t size;
        input->read((char*)&size, sizeof(size));
        IM_ASSERT(size < 1000);  // we will not use more control points, if so, then just increase the limit
        points.clear();
        points.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            ImVec2 vec;
            input->read((char*)&vec, sizeof(vec));
            vec.y = 1 - vec.y;
            points.emplace_back(vec);
        }

        refs.clear();
        refs.resize(points.size());
        std::iota(refs.begin(), refs.end(), 0);
    }

	bool CtrlPointGroup::update(ImGuiWindow* window, const bool isHovering, const ImRect& box, const ImVec2& mousePos, const float ctrlPointScale, const bool forceDisplayHandles)
    {
        const ImGuiIO& io = GetIO();
        const ImVec2 p = io.MousePos;
        const ImVec2 normalizedMouse(
            (p.x - box.Min.x) / (box.Max.x - box.Min.x),
            (p.y - box.Min.y) / (box.Max.y - box.Min.y)
        );

        // find nearest eligible control point and render all 
        int nearestEligible = -1;
        if (isHovering || forceDisplayHandles)
        {
            float minDist = std::numeric_limits<float>::max();
            for (size_t i = 0; i < refs.size(); ++i)
            {
                const size_t ref = refs[i];
                const CtrlPoint& pt = points[ref];
                const float norm2 = pt.updateHover(window, box, normalizedMouse, ctrlPointScale);
                // only select nearest if we are actually hovering and not just showing the handles
                if (isHovering && norm2 < minDist && norm2 >= 0.0f)
                {
                    nearestEligible = (int)ref;
                    minDist = norm2;
                }
            }
        }
        const bool mouseLeftClicked = io.MouseClicked[0];
        const bool mouseRightClicked = io.MouseClicked[1];
        const bool mouseLeftDown = io.MouseDown[0];

        bool hasChanged = false;

        if (selected >= 0)
        {
            if (!isHovering || !mouseLeftDown)
            {   // release point
                hasChanged = points[selected].setSelected({}, false);
                selected = -1;
            }
            else if (mouseLeftDown)
            {   // update point
                if (points[selected].setSelected(normalizedMouse, true))
                {
                    sortRefs();
                    // move except along x-axis if first or last point
                    if (selected == refs.front())
                        points[selected].pt.x = 0.0;
                    else if (selected == refs.back())
                        points[selected].pt.x = 1.0;

                    hasChanged = true;
                }
            }
        }
        else if (isHovering)
        {
            if (nearestEligible >= 0)
            {
                if (mouseLeftClicked)
                {   // set point selected
                    hasChanged = points[nearestEligible].setSelected(normalizedMouse, true);
                    selected = nearestEligible;
                }
                else if (mouseRightClicked)
                {   // delete point but only if not first or last point
                    if (nearestEligible != refs.front() && nearestEligible != refs.back())
                    {
                        points[nearestEligible].setSelected({}, false);
                        points.erase(points.begin() + nearestEligible);
                        // remove index to erased point and adapt higher indices
                        refs.erase(std::remove_if(refs.begin(), refs.end(), 
                            [nearestEligible](decltype(refs)::value_type& ref)
                            {
                                bool remove = ref == nearestEligible;
                                if (ref > nearestEligible)
                                    --ref;
                                return remove;
                            }
                        ), refs.end());

                        hasChanged = true;
                    }
                }
            }
            else if (mouseLeftClicked)
            {   // add point
                auto it = std::find_if(refs.begin(), refs.end(), [this, normalizedMouse](const size_t idx) { return normalizedMouse.x < points[idx].pt.x; });
                if (it != refs.end())
                {
                    points.emplace_back(normalizedMouse);
                    points.back().setSelected(normalizedMouse, true);
                    selected = (int)points.size() - 1;
                    refs.insert(it, selected);
                    hasChanged = true;
                }
            }
        }

        return hasChanged;
    }

    void CtrlPointGroup::sortRefs()
    {
        std::sort(refs.begin(), refs.end(), [this](const size_t a, const size_t b) -> bool { return points[a].pt.x < points[b].pt.x; });
    }

    void CtrlPointGroup::removeDuplicates()
    {
        if (size() >= 2)
        {
            // remove duplicates
            refs.erase(std::remove_if(refs.begin() + 1, refs.end(),
                [this](const size_t& ref)
                {
                    CtrlPoint& p = points[ref];
                    CtrlPoint& prevP = points[*(&ref - 1)];
                    const bool isDuplicate = p.pt.x == prevP.pt.x;
                    // if we remove a selected point move it to the previous point before deleting this index
                    // else the selection state is deleted with this point
                    if (isDuplicate && p.selected)
                        prevP = std::move(p);
                    return isDuplicate; 
                }
            ), refs.end());

            std::vector<decltype(points)::value_type> tmp;
            tmp.reserve(points.size());
            for (size_t ref : refs)
            {
                tmp.emplace_back(points[ref]);
            }

            swap(points, tmp);

            std::iota(refs.begin(), refs.end(), 0);
        }

    }

}