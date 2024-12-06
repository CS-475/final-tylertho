/*
 *  Copyright 2024 Tyler Roth
*/

#include "include/GMatrix.h"
#include "stdlib.h"
#include <cmath>
#include <iostream>

GMatrix::GMatrix() {fMat[0] = 1; fMat[2] = 0; fMat[4] = 0; 
                    fMat[1] = 0; fMat[3] = 1; fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float radians){
    float cosValue = cos(radians);
    float sinValue = sin(radians);

    return GMatrix(cosValue, -sinValue, 0, sinValue, cosValue, 0);
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix((a[0] * b[0] + a[2] * b[1]), 
                   (a[0] * b[2] + a[2] * b[3]), 
                   (a[0] * b[4] + a[2] * b[5] + a[4]),
                   (a[1] * b[0] + a[3] * b[1]),
                   (a[1] * b[2] + a[3] * b[3]),
                   (a[1] * b[4] + a[3] * b[5] + a[5]));
}

nonstd::optional<GMatrix> GMatrix::invert() const {
    float a = fMat[0];
    float b = fMat[1];
    float c = fMat[2];
    float d = fMat[3];
    float e = fMat[4];
    float f = fMat[5];

    float det = (a * d) - (b * c);

    if (det == 0) {
        return nonstd::nullopt;
    }

    float invDet = 1.0f / det;

    return GMatrix((d * invDet),   // a' = d / det
                   (-c * invDet),  // c' = -c / det
                   (c * f - d * e) * invDet,  // e' = (c * f - d * e) / det
                   (-b * invDet),  // b' = -b / det
                   (a * invDet),   // d' = a / det
                   (b * e - a * f) * invDet);
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    float a = fMat[0];
    float b = fMat[1];
    float c = fMat[2];
    float d = fMat[3];
    float e = fMat[4];
    float f = fMat[5];

    for (int i=0; i<count; i++) {
        float x = src[i].x;
        float y = src[i].y;

        dst[i].x = a * x + c * y + e;
        dst[i].y = b * x + d * y + f;
    }
}

