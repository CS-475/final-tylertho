/*
 *  Copyright 2024 Tyler Roth
 */

#include "starter_shader.h"
#include "include/GShader.h"
#include "stdlib.h"
#include "include/GMatrix.h"
#include "include/GPoint.h"
#include <cmath>
#include <iostream>
#include <vector>

bool MyShader::isOpaque() {
    return fDevice.isOpaque();
};

bool MyShader::setContext(const GMatrix& ctm) {
    fCTM = ctm;

    auto inverseTemp = (fCTM * fLocalMatrix).invert();

    if (inverseTemp) {
        fInverse = *inverseTemp;
    } else {
        return false;
    }
    return true;
};

void MyShader::shadeRow(int x, int y, int count, GPixel row[]) {
    int width = fDevice.width();
    int height = fDevice.height();

    for (int i = 0; i < count; i++) {
        GPoint dst = {x + i + 0.5f, y + 0.5f};
        GPoint src = fInverse * dst;

        int srcX, srcY;
        float fSrcX = src.x / (float) width;
        float fSrcY = src.y / (float) height;

        switch(fTileMode) {
            case GTileMode::kClamp:
                srcX = GRoundToInt(src.x);
                srcY = GRoundToInt(src.y);
                break;

            case GTileMode::kMirror:
                if ((int)floorf(fSrcX) % 2 == 0) {
                    fSrcX -= floorf(fSrcX);
                } else {
                    fSrcX = 1.0f - (fSrcX - floorf(fSrcX));
                }
                srcX = GRoundToInt(fSrcX * width);

                if ((int)floorf(fSrcY) % 2 == 0) {
                    fSrcY -= floorf(fSrcY);
                } else {
                    fSrcY = 1.0f - (fSrcY - floorf(fSrcY));
                }
                srcY = GRoundToInt(fSrcY * height);
                break;

            case GTileMode::kRepeat:
                fSrcX -= floorf(fSrcX);
                srcX = GRoundToInt(fSrcX * width);

                fSrcY -= floorf(fSrcY);
                srcY = GRoundToInt(fSrcY * height);
                break;
        }

        srcX = std::max(0, std::min(srcX, fDevice.width() - 1));
        srcY = std::max(0, std::min(srcY, fDevice.height() - 1));

        row[i] = *fDevice.getAddr(srcX, srcY);
    }
};