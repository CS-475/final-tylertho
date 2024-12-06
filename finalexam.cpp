/*
 *  Copyright 2024 Tyler Roth
*/

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include "include/GFinal.h"
#include "sweep_gradient.h"
#include "linear_gradient_pos.h"
#include "my_utils.h"
#include "stdlib.h"
#include <vector>

class FinalExam : public GFinal {
public:
    FinalExam() {}

    std::shared_ptr<GShader> createSweepGradient(GPoint center, float startRadians, const GColor colors[], int count) override {
        return std::unique_ptr<GShader>(new SweepGradient(center, startRadians, colors, count));
    }

    std::shared_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) override {
        return std::unique_ptr<GShader>(new LinearGradientPos(p0, p1, colors, pos, count));
    }

};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<GFinal>(new FinalExam());
}