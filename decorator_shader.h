/*
 *  Copyright 2024 Tyler Roth
*/

#ifndef _g_decorator_shader_h_
#define _g_decorator_shader_h_

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GPixel.h"
#include "my_utils.h"

class DecoratorShader : public GShader {
    public:
        DecoratorShader(std::shared_ptr<GShader> shader, const GMatrix& newTransformation)
            : fOgShader(shader), fNewTransformation(newTransformation) {}

        bool isOpaque() override {
            return fOgShader->isOpaque();
        }

        bool setContext(const GMatrix& ctm) override {
            return fOgShader->setContext(ctm * fNewTransformation);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override {
            fOgShader->shadeRow(x, y, count, row);
        }

    private:
        std::shared_ptr<GShader> fOgShader;
        GMatrix fNewTransformation;
};

std::shared_ptr<GShader> GCreateDecoratorShader(std::shared_ptr<GShader> shader, const GMatrix& newTransformation) {
    return std::shared_ptr<GShader>(new DecoratorShader(shader, newTransformation));
}

#endif