/*
 *  Copyright 2024 Tyler Roth
 */

#pragma once


#include "include/GTypes.h"
#include "GEdge.h"
#include "include/GColor.h"
#include "include/GPixel.h"

const int INSIDE = 0;  // 0000
const int LEFT = 1;    // 0001
const int RIGHT = 2;   // 0010
const int BOTTOM = 4;  // 0100
const int TOP = 8; 

static inline int scale_and_round_byte(float x) {
   assert(x >= 0);
   return (int)(x * 255 + 0.5f);
}

static inline GPixel color_to_pixel(GColor color) {
   GColor newcolor = color.pinToUnit();

   unsigned na = GRoundToInt(newcolor.a * 255);
   unsigned nr = GRoundToInt(newcolor.a * newcolor.r * 255);
   unsigned ng = GRoundToInt(newcolor.a * newcolor.g * 255);
   unsigned nb = GRoundToInt(newcolor.a * newcolor.b * 255);
   return GPixel_PackARGB(na, nr, ng, nb);
}

static inline GColor pixel_to_color(GPixel pixel) {
    return GColor::RGBA(
        static_cast<float>(GPixel_GetR(pixel)) / 255.0f,
        static_cast<float>(GPixel_GetG(pixel)) / 255.0f,
        static_cast<float>(GPixel_GetB(pixel)) / 255.0f,
        static_cast<float>(GPixel_GetA(pixel)) / 255.0f
    );
}

static inline int divideBy255(int value)
{
    return (value + 1 + (value >> 8)) >> 8;
}

static inline int pin(int x, int limit) {
    return std::min(std::max(0, x), limit);
}

// turn 0xAABBCCDD into 0x00AA00CC00BB00DD
static inline uint64_t extend(uint32_t x) {
    uint64_t hi = x & 0xFF00FF00;  // A and G components
    uint64_t lo = x & 0x00FF00FF;  // R and B components
    return (hi << 24) | lo;
}

// turn 0xXX into 0x00XX00XX00XX00XX
static inline uint64_t duplicate(uint64_t x) {
    return (x << 48) | (x << 32) | (x << 16) | x;
}

// turn 0x..AA..CC..BB..DD into 0xAABBCCDD
static inline uint32_t compact(uint64_t x) {
    return ((x >> 24) & 0xFF00FF00) | (x & 0xFF00FF);
}

static inline uint32_t quad_mul_div255(uint32_t x, uint8_t invA) {
    // auto r = divideBy255(GPixel_GetR(x) * invA);
    // auto g = divideBy255(GPixel_GetG(x) * invA);
    // auto b = divideBy255(GPixel_GetB(x) * invA);
    // auto a = divideBy255(GPixel_GetA(x) * invA);

    // return GPixel_PackARGB(a, r, g, b);

    uint64_t prod = extend(x) * invA;
    prod += duplicate(128);			
    prod += (prod >> 8) & duplicate(0xFF);
    prod >>= 8;
    return compact(prod);
}