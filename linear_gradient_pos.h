/*
 *  Copyright 2024 Tyler Roth
*/

#ifndef _g_linear_gradient_pos_h_
#define _g_linear_gradient_pos_h_

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include "my_utils.h"
#include "stdlib.h"
#include <vector>

class LinearGradientPos : public GShader {
    public:
        LinearGradientPos(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) 
            : fColors(colors, colors + count), fNumColors(count), fPositions(pos, pos + count) {

        for (int i = 0; i < count; ++i) {
            fDiffColors.push_back(colors[i+1] - colors[i]);
        }

        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;

        fUnitMatrix = { dx, -dy, p0.x, dy, dx, p0.y };
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

            for (int i = 0; i < count; ++i) {
                float t = dst.x;
                t = std::max(0.0f, std::min(1.0f, t));

                int index = 0;
                while (index < fNumColors - 1 && t > fPositions[index + 1]) {
                    ++index;
                }
                // need to change t

                float start = fPositions[index];
                float end = fPositions[index + 1];
                float segmentLength = end - start;
                float currT = (t - start) / (segmentLength);
                
                GColor color = fColors[index] + (fDiffColors[index] * currT);

                row[i] = color_to_pixel(color);

                dst.x += fInverseCTM[0];
            }
        }


    private:
        const GBitmap fDevice;
        std::vector<GColor> fDiffColors;
        std::vector<GColor> fColors;
        std::vector<float> fPositions;
        GMatrix fInverseCTM;
        GMatrix fUnitMatrix;
        int fNumColors;

        float linearClamp(float px) {
            return std::max(0.0f, std::min(px, 1.0f)) * (fNumColors - 1);
        }

};

#endif