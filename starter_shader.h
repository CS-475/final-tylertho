/*
 *  Copyright 2024 Tyler Roth
 */

#ifndef _g_starter_shader_h_
#define _g_starter_shader_h_

#include "include/GShader.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GPixel.h"
#include "include/GMatrix.h"
#include "include/GPoint.h"
#include "include/GBlendMode.h"
#include "my_utils.h"

class MyShader : public GShader {
public:
    MyShader(const GBitmap& device, const GMatrix& localMatrix, GTileMode tileMode) 
        : fDevice(device), fLocalMatrix(localMatrix), fTileMode(tileMode) {}
    // Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override;

    // The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override;

    /**
     *  Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
     *  corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
     *  can hold at least [count] entries.
     */
    void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    GMatrix fLocalMatrix;
    GMatrix fCTM; 
    GMatrix fInverse;
    GTileMode fTileMode;
};

/**
    *  Return a subclass of GShader that draws the specified bitmap and the local matrix.
    *  Returns null if the subclass can not be created.
*/
std::shared_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode mode) {
    if (!bitmap.pixels()) {
        return nullptr;
    }
    return std::make_shared<MyShader>(bitmap, localMatrix, mode);
}

#endif