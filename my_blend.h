/*
 *  Copyright 2024 Tyler Roth
 */

#include "include/GBlendMode.h"
#include "include/GColor.h"
#include "include/GPixel.h"
// #include "my_utils.h"
#include "stdlib.h"
#include <iostream>

static inline int multiplyBytes(int x, int y) {
    int result = x * y;

    return (result + 127) / 255;
}

GPixel kSrcOver(GPixel src, GPixel dst);
GPixel kDstOver(GPixel src, GPixel dst);
GPixel kSrcIn(GPixel src, GPixel dst);
GPixel kDstIn(GPixel src, GPixel dst);
GPixel kSrcOut(GPixel src, GPixel dst);
GPixel kDstOut(GPixel src, GPixel dst);
GPixel kSrcATop(GPixel src, GPixel dst);
GPixel kDstATop(GPixel src, GPixel dst);
GPixel kXor(GPixel src, GPixel dst);

GPixel kSrcOver(GPixel src, GPixel dst) {
    // float soalpha = (255.0f - GPixel_GetA(src))/255.0f;
    // unsigned soA = GRoundToInt(GPixel_GetA(src) + (soalpha * GPixel_GetA(dst))); 
    // unsigned soR = GRoundToInt(GPixel_GetR(src) + (soalpha * GPixel_GetR(dst)));
    // unsigned soG = GRoundToInt(GPixel_GetG(src) + (soalpha * GPixel_GetG(dst))); 
    // unsigned soB = GRoundToInt(GPixel_GetB(src) + (soalpha * GPixel_GetB(dst))); 
    // return GPixel_PackARGB(soA, soR, soG, soB);
    return src + kDstOut(src, dst);
}

GPixel kDstOver(GPixel src, GPixel dst) {
    // float doalpha = (255.0f - GPixel_GetA(dst))/255.0f;
    // unsigned doA = GRoundToInt(GPixel_GetA(dst) + (doalpha * GPixel_GetA(src)));
    // unsigned doR = GRoundToInt(GPixel_GetR(dst) + (doalpha * GPixel_GetR(src))); 
    // unsigned doG = GRoundToInt(GPixel_GetG(dst) + (doalpha * GPixel_GetG(src))); 
    // unsigned doB = GRoundToInt(GPixel_GetB(dst) + (doalpha * GPixel_GetB(src)));
    // return GPixel_PackARGB(doA, doR, doG, doB);
    return dst + kSrcOut(src, dst);     
}

GPixel kSrcIn(GPixel src, GPixel dst) {
    // float sidst_a = (GPixel_GetA(dst)/255.0f);
    // unsigned siA = GRoundToInt(sidst_a * GPixel_GetA(src)); 
    // unsigned siR = GRoundToInt(sidst_a * GPixel_GetR(src)); 
    // unsigned siG = GRoundToInt(sidst_a * GPixel_GetG(src)); 
    // unsigned siB = GRoundToInt(sidst_a * GPixel_GetB(src)); 
    // return GPixel_PackARGB(siA, siR, siG, siB);
    const int dst_a = GPixel_GetA(dst);

    return quad_mul_div255(src, dst_a);
}

GPixel kDstIn(GPixel src, GPixel dst) {
    // float disrc_a = (GPixel_GetA(src)/255.0f);
    // unsigned diA = GRoundToInt(disrc_a * GPixel_GetA(dst)); 
    // unsigned diR = GRoundToInt(disrc_a * GPixel_GetR(dst)); 
    // unsigned diG = GRoundToInt(disrc_a * GPixel_GetG(dst)); 
    // unsigned diB = GRoundToInt(disrc_a * GPixel_GetB(dst)); 
    // return GPixel_PackARGB(diA, diR, diG, diB);
    const int src_a = GPixel_GetA(src);

    return quad_mul_div255(dst, src_a);
}

GPixel kSrcOut(GPixel src, GPixel dst) {
    // float sout_dst_a = (255.0f- GPixel_GetA(dst))/255.0f;
    // unsigned sout_A = GRoundToInt(sout_dst_a * GPixel_GetA(src)); 
    // unsigned sout_R = GRoundToInt(sout_dst_a * GPixel_GetR(src)); 
    // unsigned sout_G = GRoundToInt(sout_dst_a * GPixel_GetG(src)); 
    // unsigned sout_B = GRoundToInt(sout_dst_a * GPixel_GetB(src));
    // return GPixel_PackARGB(sout_A, sout_R, sout_G, sout_B);
    return quad_mul_div255(src, 255 - GPixel_GetA(dst));
}

GPixel kDstOut(GPixel src, GPixel dst) {
    // float dout_src_a = (255.0f- GPixel_GetA(src))/255.0f; 
    // unsigned dout_A = GRoundToInt(dout_src_a * GPixel_GetA(dst)); 
    // unsigned dout_R = GRoundToInt(dout_src_a * GPixel_GetR(dst)); 
    // unsigned dout_G = GRoundToInt(dout_src_a * GPixel_GetG(dst)); 
    // unsigned dout_B = GRoundToInt(dout_src_a * GPixel_GetB(dst)); 
    // return GPixel_PackARGB(dout_A, dout_R, dout_G, dout_B);
    return quad_mul_div255(dst, 255 - GPixel_GetA(src));
}

GPixel kSrcATop(GPixel src, GPixel dst) {
    // unsigned srcA = GPixel_GetA(src);
    // unsigned srcR = GPixel_GetR(src);
    // unsigned srcG = GPixel_GetG(src);
    // unsigned srcB = GPixel_GetB(src);

    // unsigned dstA = GPixel_GetA(dst);
    // unsigned dstR = GPixel_GetR(dst);
    // unsigned dstG = GPixel_GetG(dst);
    // unsigned dstB = GPixel_GetB(dst);

    // unsigned resultA = dstA;
    // unsigned resultR = multiplyBytes(resultA, srcR) + multiplyBytes(255 - srcA, dstR);
    // unsigned resultG = multiplyBytes(resultA, srcG) + multiplyBytes(255 - srcA, dstG);
    // unsigned resultB = multiplyBytes(resultA, srcB) + multiplyBytes(255 - srcA, dstB);

    // return GPixel_PackARGB(resultA, resultR, resultG, resultB);
    return kSrcIn(src, dst) + kDstOut(src, dst);
}

GPixel kDstATop(GPixel src, GPixel dst) {
    // unsigned srcA = GPixel_GetA(src);
    // unsigned srcR = GPixel_GetR(src);
    // unsigned srcG = GPixel_GetG(src);
    // unsigned srcB = GPixel_GetB(src);

    // unsigned dstA = GPixel_GetA(dst);
    // unsigned dstR = GPixel_GetR(dst);
    // unsigned dstG = GPixel_GetG(dst);
    // unsigned dstB = GPixel_GetB(dst);

    // unsigned resultA = srcA;
    // unsigned resultR = multiplyBytes(resultA, dstR) + multiplyBytes(255 - dstA, srcR);
    // unsigned resultG = multiplyBytes(resultA, dstG) + multiplyBytes(255 - dstA, srcG);
    // unsigned resultB = multiplyBytes(resultA, dstB) + multiplyBytes(255 - dstA, srcB);

    // return GPixel_PackARGB(resultA, resultR, resultG, resultB);
    return kDstIn(src, dst) + kSrcOut(src, dst);
}

GPixel kXor(GPixel src, GPixel dst) {
    // float kx_dst_a = (255.0f - GPixel_GetA(dst))/255.0f; 
    // float kx_src_a = (255.0f - GPixel_GetA(src))/255.0f;
    // unsigned kx_A = GRoundToInt(kx_src_a * GPixel_GetA(dst) + kx_dst_a * GPixel_GetA(src)); 
    // unsigned kx_R = GRoundToInt(kx_src_a * GPixel_GetR(dst) + kx_dst_a * GPixel_GetR(src));
    // unsigned kx_G = GRoundToInt(kx_src_a * GPixel_GetG(dst) + kx_dst_a * GPixel_GetG(src)); 
    // unsigned kx_B = GRoundToInt(kx_src_a * GPixel_GetB(dst) + kx_dst_a * GPixel_GetB(src));
    // return GPixel_PackARGB(kx_A, kx_R, kx_G, kx_B);
    return kDstOut(src, dst) + kSrcOut(src, dst);
}

GPixel blendColors(const GPixel src, GPixel dst, GBlendMode mode) {
    switch (mode) {
        case(GBlendMode::kClear):
            return GPixel_PackARGB(0, 0, 0, 0);
        case(GBlendMode::kSrc):
            return src;
        case(GBlendMode::kDst):
            return dst;
        case(GBlendMode::kSrcOver):
            return kSrcOver(src, dst);
        case(GBlendMode::kDstOver):
            return kDstOver(src, dst);
        case(GBlendMode::kSrcIn):
            return kSrcIn(src, dst);
        case(GBlendMode::kDstIn):
            return kDstIn(src, dst);
        case(GBlendMode::kSrcOut):
            return kSrcOut(src, dst);
        case(GBlendMode::kDstOut):
            return kDstOut(src, dst);
        case(GBlendMode::kSrcATop):
            return kSrcATop(src, dst);
        case(GBlendMode::kDstATop):
            return kDstATop(src, dst);
        case(GBlendMode::kXor):
            return kXor(src, dst);
    }
    return GPixel_PackARGB(0, 0, 0, 0);
}