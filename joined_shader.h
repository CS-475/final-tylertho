/*
 *  Copyright 2024 Tyler Roth
*/

#ifndef _g_joined_shader_h_
#define _g_joined_shader_h_

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GPixel.h"
#include "my_utils.h"

class JoinedShader : public GShader {
    public:
        JoinedShader(std::shared_ptr<GShader> shader1, std::shared_ptr<GShader> shader2)
            : fShader1(std::move(shader1)), fShader2(std::move(shader2)) {}

        bool isOpaque() override {
            return fShader1->isOpaque() && fShader2->isOpaque(); 
        }

        bool setContext(const GMatrix& ctm) override {
            return fShader1->setContext(ctm) && fShader2->setContext(ctm);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override {
            GPixel row1[count];
            GPixel row2[count];

            fShader1->shadeRow(x, y, count, row1);
            fShader2->shadeRow(x, y, count, row2);

            for (int i=0; i<count; i++) {
                row[i] = multiplyPixelValues(row1[i], row2[i]);
            }
        }


    private:
        std::shared_ptr<GShader> fShader1;
        std::shared_ptr<GShader> fShader2;

        GPixel multiplyPixelValues(GPixel pixel1, GPixel pixel2) {
            int a = GRoundToInt((GPixel_GetA(pixel1) * GPixel_GetA(pixel2)) / 255.0f);
            int r = GRoundToInt((GPixel_GetR(pixel1) * GPixel_GetR(pixel2)) / 255.0f);
            int g = GRoundToInt((GPixel_GetG(pixel1) * GPixel_GetG(pixel2)) / 255.0f);
            int b = GRoundToInt((GPixel_GetB(pixel1) * GPixel_GetB(pixel2)) / 255.0f);
            
            return GPixel_PackARGB(a, r, g, b);
        }
};

std::shared_ptr<GShader> GCreateJoinedShader(std::shared_ptr<GShader> shader1, std::shared_ptr<GShader> shader2) {
    return std::shared_ptr<GShader>(new JoinedShader(shader1, shader2));
}

#endif