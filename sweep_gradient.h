/*
 *  Copyright 2024 Tyler Roth
*/

#ifndef _g_sweep_shader_h_
#define _g_sweep_shader_h_

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include "my_utils.h"
#include "stdlib.h"
#include <vector>

class SweepGradient : public GShader {
    public:
        SweepGradient(GPoint center, float startRadians, const GColor colors[], int count) : 
            fColors(colors, colors + count), fNumColors(count) {
                fCenter = center;
                fStartRadians = startRadians;
        }

        bool isOpaque() override {
            for (int i = 0; i < fNumColors; i++) {
                if (fColors[i].a != 1.0) {
                    return false;
                }
            }
            return true;    
        };

        bool setContext(const GMatrix& ctm) override {
            auto inverseTemp = (ctm * fUnitMatrix).invert();

            if (inverseTemp) {
                fInverseCTM = *inverseTemp;
            } else {
                return false;
            }
            return true;
        };

        void shadeRow(int x, int y, int count, GPixel row[]) override {
            GPoint point = { x + 0.5f, y + 0.5f };
            GPoint dst = fInverseCTM * point;
            GPoint center = fInverseCTM * fCenter;
            float dx = fInverseCTM[0];


            if (fNumColors == 1) {
                for (int i = 0; i < count; ++i) {
                    row[i] = color_to_pixel(fColors[0]);
                }
            } else {
                for (int i = 0; i < count; ++i) {
                    // arctan to find angle of point to center
                    float angle = atan2(point.y - center.y, point.x - center.x) - fStartRadians;

                    if (angle < 0) {
                        angle += 2*M_PI;
                    }

                    float t = angle / (2 * M_PI);

                    int color = GFloorToInt((fNumColors - 1) * t);
                    float fraction = 1.0f / (fNumColors - 1);
                    float newcolor = color * fraction;

                    t = GPinToUnit((t - newcolor) / fraction);
                    row[i] = color_to_pixel(fColors[color] * (1.f - t) + fColors[color + 1] * t);
                    point.x += dx;
                }
            }
        }


    private:
        GPoint fCenter;
        std::vector<GColor> fDiffColors;
        std::vector<GColor> fColors;
        int fNumColors;
        float fStartRadians;
        GMatrix fInverseCTM;
        GMatrix fUnitMatrix;

};

#endif