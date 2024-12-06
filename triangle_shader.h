/*
 *  Copyright 2024 Tyler Roth
*/

#ifndef _g_triangle_shader_h_
#define _g_triangle_shader_h_

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include "my_utils.h"

class TriangleShader : public GShader {
    public:
        TriangleShader(const GPoint points[3], const GColor colors[]) : fColors(colors, colors + 3) {
            GPoint u = points[1] - points[0];
            GPoint v = points[2] - points[0];

            fUnitMatrix = { u.x, v.x, points[0].x, 
                            u.y, v.y, points[0].y };

            colorDiff1 = colors[1] - colors[0];
            colorDiff2 = colors[2] - colors[0];
        }

        bool isOpaque() override {
            return fColors[0].a == 1.0 && fColors[1].a == 1.0 && fColors[2].a == 1.0;
        }

        bool setContext(const GMatrix& ctm) override {
            auto inverseTemp = (ctm * fUnitMatrix).invert();

            if (inverseTemp) {
                fInverse = *inverseTemp;
            } else {
                return false;
            }
            return true;
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override {
            GPoint dst = { x + 0.5f, y + 0.5f };
            GPoint src = fInverse * dst;

            GColor color = src.x * colorDiff1 + src.y * colorDiff2 + fColors[0];
            GColor colorChange = fInverse[0] * colorDiff1 + fInverse[1] * colorDiff2;
            
            for (int i = 0; i < count; ++i) {
                row[i] = color_to_pixel(color);
                
                color += colorChange;
            }
        }
    
    private:
        GMatrix fInverse, fUnitMatrix;
        GColor colorDiff1, colorDiff2;
        std::vector<GColor> fColors;
        int fNumColors;
};

std::shared_ptr<GShader> GCreateTriangleShader(const GPoint points[3], const GColor colors[]) {
    return std::shared_ptr<GShader>(new TriangleShader(points, colors));
}

#endif