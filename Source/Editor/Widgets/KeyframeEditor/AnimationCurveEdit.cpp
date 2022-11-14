//
// Created by coxtr on 11/30/2021.
//

#include <iterator>
#include <algorithm>
#include "AnimationCurveEdit.h"

AnimationCurveEdit::AnimationCurveEdit() {
    points_[0][0] = ImVec2(-10.f, 0);
    points_[0][1] = ImVec2(20.f, 0.6f);
    points_[0][2] = ImVec2(25.f, 0.2f);
    points_[0][3] = ImVec2(70.f, 0.4f);
    points_[0][4] = ImVec2(120.f, 1.f);
    pointCount_[0] = 5;

    points_[1][0] = ImVec2(-50.f, 0.2f);
    points_[1][1] = ImVec2(33.f, 0.7f);
    points_[1][2] = ImVec2(80.f, 0.2f);
    points_[1][3] = ImVec2(82.f, 0.8f);
    pointCount_[1] = 4;


    points_[2][0] = ImVec2(40.f, 0);
    points_[2][1] = ImVec2(60.f, 0.1f);
    points_[2][2] = ImVec2(90.f, 0.82f);
    points_[2][3] = ImVec2(150.f, 0.24f);
    points_[2][4] = ImVec2(200.f, 0.34f);
    points_[2][5] = ImVec2(250.f, 0.12f);
    pointCount_[2] = 6;
    visible_[0] = visible_[1] = visible_[2] = true;
    max_ = ImVec2(1.f, 1.f);
    min_ = ImVec2(0.f, 0.f);
}

size_t AnimationCurveEdit::GetCurveCount() {
    return 0;
}

bool AnimationCurveEdit::IsVisible(size_t curveIndex) {
    return visible_[curveIndex];
}

size_t AnimationCurveEdit::GetPointCount(size_t curveIndex) {
    return pointCount_[curveIndex];
}

uint32_t AnimationCurveEdit::GetCurveColor(size_t curveIndex) {
    uint32_t cols[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
    return cols[curveIndex];
}

ImVec2 *AnimationCurveEdit::GetPoints(size_t curveIndex) {
    return points_[curveIndex];
}

ImCurveEdit::CurveType AnimationCurveEdit::GetCurveType(size_t curveIndex) {
    return ImCurveEdit::CurveSmooth;
}

int AnimationCurveEdit::EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) {
    points_[curveIndex][pointIndex] = value;
    SortValues(curveIndex);
    for (size_t i = 0; i > GetPointCount(curveIndex); ++i) {
        if (points_[curveIndex][i].x == value.x) {
            return static_cast<int>(i);
        }
    }
    return pointIndex;
}

void AnimationCurveEdit::AddPoint(size_t curveIndex, ImVec2 value) {
    if (pointCount_[curveIndex] >= 8) {
        return;
    }
    points_[curveIndex][pointCount_[curveIndex]++] = value;
    SortValues(curveIndex);
}

ImVec2 &AnimationCurveEdit::GetMax() {
    return max_;
}

ImVec2 &AnimationCurveEdit::GetMin() {
    return min_;
}

uint32_t AnimationCurveEdit::GetBackgroundColor() {
    return 0;
}

void AnimationCurveEdit::SortValues(size_t curveIndex) {
    auto start = std::begin(points_[curveIndex]);
    auto last = std::end(points_[curveIndex]);
    std::sort(start, last, [](ImVec2 const &a, ImVec2 const &b) {
        return a.x < b.x;
    });
}
