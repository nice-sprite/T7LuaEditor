//
// Created by coxtr on 11/30/2021.
//

#ifndef T7LUAEDITOR_ANIMATIONCURVEEDIT_H
#define T7LUAEDITOR_ANIMATIONCURVEEDIT_H

#include <ImCurveEdit.h>

class AnimationCurveEdit : public ImCurveEdit::Delegate {
    friend class AnimationTimelineSequencer;

private:
    ImVec2 points_[3][8];
    size_t pointCount_[3];
    bool visible_[3];
    ImVec2 min_, max_;

public:

    AnimationCurveEdit();

    size_t GetCurveCount() override;

    bool IsVisible(size_t curveIndex) override;

    size_t GetPointCount(size_t curveIndex) override;

    uint32_t GetCurveColor(size_t curveIndex) override;

    ImVec2 *GetPoints(size_t curveIndex) override;

    virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex);

    int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override;

    void AddPoint(size_t curveIndex, ImVec2 value) override;

    ImVec2 &GetMax() override;

    ImVec2 &GetMin() override;

    uint32_t GetBackgroundColor() override;


private:
    void SortValues(size_t curveIndex);


};


#endif //T7LUAEDITOR_ANIMATIONCURVEEDIT_H
