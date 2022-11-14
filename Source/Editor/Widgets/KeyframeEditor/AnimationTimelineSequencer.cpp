//
// Created by coxtr on 11/29/2021.
//

#include "AnimationTimelineSequencer.h"

std::array<std::string, 1> AnimationTimelineSequencer::SequencerItemNames = {"Animations"s};

AnimationTimelineSequencer::AnimationTimelineSequencer() : frameMin_(0), frameMax_(0) {

}

int AnimationTimelineSequencer::GetFrameMin() const {
    return frameMin_;
}

int AnimationTimelineSequencer::GetFrameMax() const {
    return frameMax_;
}

int AnimationTimelineSequencer::GetItemCount() const {
    return static_cast<int>(items_.size());
}

int AnimationTimelineSequencer::GetItemTypeCount() const {
    return sizeof(SequenceItem) / sizeof(char *);
}

const char *AnimationTimelineSequencer::GetItemTypeName(int typeIndex) const {
    return SequencerItemNames[typeIndex].c_str();
}

const char *AnimationTimelineSequencer::GetItemLabel(int index) const {
    static auto str = fmt::format("[{}] {}", index, SequencerItemNames.at(index));
    return str.c_str();
}

void AnimationTimelineSequencer::Get(int index, int **start, int **end, int *type, unsigned int *color) {
    SequenceItem &item = items_.at(index);
    if (color)
        *color = 0xFFAA8080; // same color for everyone, return color based on type
    if (start)
        *start = &item.frameStart;
    if (end)
        *end = &item.frameEnd;
    if (type)
        *type = item.type;
}

void AnimationTimelineSequencer::Add(int type) {
    items_.emplace_back(type, 0, 10, false);
}

void AnimationTimelineSequencer::Del(int index) {
    items_.erase(std::begin(items_) + index);
}

void AnimationTimelineSequencer::Duplicate(int index) {
    items_.push_back(items_.at(index));
}

size_t AnimationTimelineSequencer::GetCustomHeight(int index) {
    return items_.at(index).expanded ? 300 : 0;
}

void AnimationTimelineSequencer::DoubleClick(int index) {
    if (items_.at(index).expanded) {
        items_.at(index).expanded = false;
        return;
    }
    for (auto &item: items_)
        item.expanded = false;
    items_.at(index).expanded = !items_.at(index).expanded;
}

void AnimationTimelineSequencer::CustomDraw(int index, ImDrawList *drawList, const ImRect &rc, const ImRect &legendRec,
                                            const ImRect &clippingRect,
                                            const ImRect &legendClippingRect) {

    static std::array<const char *, 3> labels = {"Translation", "Rotation", "Scale"};
    curveEdit.max_ = ImVec2(float(frameMax_), 1.f);
    curveEdit.min_ = ImVec2(float(frameMin_), 0.f);
    drawList->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
    for (int i = 0; i < 3; ++i) {
        ImVec2 pta(legendRec.Min.x + 30, legendRec.Min.y + i * 14.f);
        ImVec2 ptb(legendRec.Max.x + 30, legendRec.Min.y + (i + 1) * 14.f);
        drawList->AddText(pta, curveEdit.visible_[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
        if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
            curveEdit.visible_[i] = !curveEdit.visible_[i];
    }
    drawList->PopClipRect();
    ImGui::SetCursorScreenPos(rc.Min);

    ImCurveEdit::Edit(curveEdit, rc.GetSize(), index, &clippingRect);

}

void AnimationTimelineSequencer::CustomDrawCompact(int index, ImDrawList *drawList, const ImRect &rc,
                                                   const ImRect &clippingRect) {

    curveEdit.max_ = ImVec2(float(frameMax_), 1.f);
    curveEdit.min_ = ImVec2(float(frameMin_), 0.f);
    drawList->PushClipRect(clippingRect.Min, clippingRect.Max, true);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < curveEdit.pointCount_[i]; j++) {
            float p = curveEdit.points_[i][j].x;
            if (p < items_.at(index).frameStart || p > items_.at(index).frameEnd)
                continue;
            float r = (p - frameMin_) / float(frameMax_ - frameMin_);
            float x = ImLerp(rc.Min.x, rc.Max.x, r);
            drawList->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
        }
    }
    drawList->PopClipRect();
}
