/*
 *  Copyright 2024 Tyler Roth
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GShader.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GPixel.h"
#include "triangle_shader.h"
#include "decorator_shader.h"
#include "joined_shader.h"
#include "my_utils.h"
#include "stdlib.h"
#include <stack>

class MyCanvas : public GCanvas {
public:
    // MyCanvas(const GBitmap& device) : fDevice(device) {}
    MyCanvas(const GBitmap& device);

    void save() override;
    void restore() override;
    void concat(const GMatrix& matrix) override;
    void clear(const GColor& color) override;
    void fillRect(const GRect& rect, const GColor& color);
    void drawRect(const GRect&, const GPaint&) override;
    void drawConvexPolygon(const GPoint[], int count, const GPaint& paint) override;
    void blit(const int y, const int xLeft, const int xRight, const GPaint& paint);
    void pathScan(std::vector<Edge> edges, GShader* shader, GPaint paint, GPixel src);
    void drawPath(const GPath&, const GPaint&);
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint&);
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint&);

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    GMatrix fCTM;
    std::stack<GMatrix> fSaveStack;

    // Add whatever other fields you need

    GMatrix compute_basis(const GPoint pts[3]) {
        return { pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x,
             pts[1].y - pts[0].y, pts[2].y - pts[0].y, pts[0].y };
    }

    void drawTriangle(const GPoint pts[3], GPaint paint) {
        drawConvexPolygon(pts, 3, paint);
    }

    void drawTriangleWithTex(const GPoint pts[3], const GPoint tex[3], std::shared_ptr<GShader> originalShader) {
        GMatrix P, T, invT;

        P = compute_basis(pts);
        T = compute_basis(tex);

        auto optInvT = T.invert();
        if (optInvT) {
            invT = *optInvT;
        } else {
            return;
        }

        std::shared_ptr<GShader> proxy = std::make_shared<DecoratorShader>(std::shared_ptr<GShader>(originalShader), P * invT);
        GPaint paint(proxy);
        drawTriangle(pts, paint);
    }

    void drawTriangleWithTexColor(const GPoint pts[3], const GColor colors[3], const GPoint tex[3], std::shared_ptr<GShader> originalShader) {
        GMatrix P, T, invT;

        P = compute_basis(pts);
        T = compute_basis(tex);

        auto optInvT = T.invert();
        if (optInvT) {
            invT = *optInvT;
        } else {
            return;
        }

        std::shared_ptr<GShader> proxy = std::make_shared<DecoratorShader>(originalShader, P * invT);
        std::shared_ptr<GShader> triangle = std::make_shared<TriangleShader>(pts, colors);
        std::shared_ptr<GShader> combined = std::make_shared<JoinedShader>(proxy, triangle);        
        drawTriangle(pts, GPaint(combined));
    }

    GColor getQuadColor(const GColor colors[4], float u, float v) {
        float a = ((1 - v) * (1 - u) * colors[0].a) + (u * (1 - v) * colors[1].a) + (v * (1 - u) * colors[3].a) + (u * v * colors[2].a);
        float r = ((1 - v) * (1 - u) * colors[0].r) + (u * (1 - v) * colors[1].r) + (v * (1 - u) * colors[3].r) + (u * v * colors[2].r);
        float g = ((1 - v) * (1 - u) * colors[0].g) + (u * (1 - v) * colors[1].g) + (v * (1 - u) * colors[3].g) + (u * v * colors[2].g);
        float b = ((1 - v) * (1 - u) * colors[0].b) + (u * (1 - v) * colors[1].b) + (v * (1 - u) * colors[3].b) + (u * v * colors[2].b);

        return GColor::RGBA(r, g, b, a); 
    }

    GPoint getQuadrilateralPoint(const GPoint points[4], float u, float v) {
        return ((1 - v) * (1 - u) * points[0]) + (u * (1 - v) * points[1]) + (v * (1 - u) * points[3]) + (u * v * points[2]);
    }
};


#endif
