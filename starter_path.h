/**
 *  Copyright 2024 Tyler Roth
 */

#ifndef _g_starter_path_h_
#define _g_starter_path_h_

#include "include/GPoint.h"
#include "include/GPath.h"

struct QuadraticCurve {
    float AB;
    float BC;
    float ABC;
};

struct CubicCurve {
    QuadraticCurve ABC;
    QuadraticCurve BCD;
    float ABCD;
};

QuadraticCurve getQuadPoint(float A, float B, float C, float t);

CubicCurve getCubicPoint(float A, float B, float C, float D, float t);

#endif