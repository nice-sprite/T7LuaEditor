//
// Created by coxtr on 11/29/2021.
//

#ifndef T7LUAEDITOR_ANIMATIONTIMELINESEQUENCER_H
#define T7LUAEDITOR_ANIMATIONTIMELINESEQUENCER_H

#include <imgui.h>
#include <imgui_internal.h>
#include <ImSequencer.h>
#include <vector>
#include "AnimationCurveEdit.h"
#include <string>
#include <array>
#include <fmt/core.h>

using namespace std::string_literals;

class AnimationTimelineSequencer : public ImSequencer::SequenceInterface {
private:
    static std::array<std::string, 1> SequencerItemNames;
    AnimationCurveEdit curveEdit;
public:
    struct SequenceItem {
        int type;
        int frameStart, frameEnd;
        bool expanded;

        SequenceItem() = default;

        SequenceItem(int _type, int _frameStart, int _frameEnd, bool _expanded) :
                type(_type), frameStart(_frameStart), frameEnd(_frameEnd), expanded(_expanded) {}
    };

    int frameMin_, frameMax_;
    std::vector<SequenceItem> items_;

    AnimationTimelineSequencer();

    int GetFrameMin() const override;

    int GetFrameMax() const override;

    int GetItemCount() const override;

    int GetItemTypeCount() const;

    void Get(int index, int **start, int **end, int *type, unsigned int *color) override;

    const char *GetItemTypeName(int typeIndex) const override;

    const char *GetItemLabel(int index) const override;

    void Add(int type);

    void Del(int index);

    void Duplicate(int index);

    size_t GetCustomHeight(int index);

    void DoubleClick(int index);

    void
    CustomDraw(int index, ImDrawList *drawList, const ImRect &rc, const ImRect &legendRec, const ImRect &clippingRect,
               const ImRect &legendClippingRect);

    void CustomDrawCompact(int index, ImDrawList *drawList, const ImRect &rc, const ImRect &clippingRect);
};


#endif //T7LUAEDITOR_ANIMATIONTIMELINESEQUENCER_H
