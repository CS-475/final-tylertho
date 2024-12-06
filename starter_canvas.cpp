/*
 *  Copyright 2024 Tyler Roth
 */

#include "starter_canvas.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include "include/GPathBuilder.h"
#include "stdlib.h"
#include "include/GBlendMode.h"
#include "my_blend.h"
#include "GEdge.h"
#include <iostream>
#include <vector>

MyCanvas::MyCanvas(const GBitmap& device) : fDevice(device) {
    fCTM = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    fSaveStack.push(fCTM);  
}

void MyCanvas::save() {
    fSaveStack.push(fCTM);
};

void MyCanvas::restore() {
    assert(!fSaveStack.empty());
    fCTM = fSaveStack.top();
    fSaveStack.pop();
};

void MyCanvas::concat(const GMatrix& matrix) {
    fCTM = GMatrix::Concat(fCTM, matrix);
};


void MyCanvas::clear(const GColor& color) {
    GPixel *row_address;
    int height = fDevice.height();
    int width = fDevice.width();
    GPixel newColor = color_to_pixel(color);

    for (int y=0; y<height; y++) {
        row_address = fDevice.getAddr(0, y);
        for (int x=0; x<width; x++) {
            row_address[x] = newColor;
        }
    }
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    GPoint points[4] = {
        {rect.right, rect.top},
        {rect.right, rect.bottom},
        {rect.left, rect.bottom},
        {rect.left, rect.top}
    };

    drawConvexPolygon(points, 4, paint);
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
    // Ensure the polygon has at least 3 points to form a valid shape
    if (count < 3) {
        return;
    }

    // put points back
    std::vector<Edge> edges;

    GShader* shader = paint.peekShader();

    if (shader) {
        shader->setContext(fCTM);
    }
    
    GPoint dstPoints[count];
    fCTM.mapPoints(dstPoints, points, count);

    edges = buildEdges(fDevice, count, dstPoints);

    if (edges.size() < 2) {
        return;
    }

    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        // case if tops are the same
        if (a.y0 == b.y0) {
            return a.x0 < b.x0;
        }
        return a.y0 < b.y0;
    });

    // std::sort(edges.begin(), edges.end(), sortEdges);

    if (edges.empty()) {
        return;
    }

    for (int y = std::max(0, edges.front().y0); y < std::min(fDevice.height(), edges.back().y1); ++y) {
        std::vector<float> xIntersections;
        int fy = GRoundToInt(y + 0.5f);
        for (const Edge& edge : edges) {
            if (y >= edge.y0 && y < edge.y1) {
                // float x = edge.start.x + (fy - edge.start.y) * edge.m;
                float x = edge.computeX(fy);
                xIntersections.push_back(x);
            }
        }

        std::sort(xIntersections.begin(), xIntersections.end());

        for (size_t i = 0; i < xIntersections.size(); i += 2) {
            int x1 = pin(GRoundToInt(xIntersections[i]), fDevice.width());
            int x2 = pin(GRoundToInt((xIntersections[i + 1])), fDevice.width());

            blit(y, x1, x2, paint);
        }
    }
}

void MyCanvas::blit(int y, int xLeft, int xRight, const GPaint& paint) {
    xLeft = std::max(0, xLeft);
    xRight = std::min(fDevice.width(), xRight);

    GBlendMode blendMode = paint.getBlendMode();

    GShader* shader = paint.peekShader();
    if (shader == nullptr) {
        GColor color = paint.getColor().pinToUnit();
        GPixel srcPixel = color_to_pixel(color);

        for (int x = xLeft; x < xRight; ++x) {
            GPixel* addr = fDevice.getAddr(x, y);

            GPixel blendedPixel = blendColors(srcPixel, *addr, blendMode);
            
            *addr = blendedPixel;
        }
    } else {
        if (!shader->setContext(fCTM)) {
            return;
        }

        if (xRight < xLeft) {
            std::swap(xRight, xLeft);
        }

        assert(xLeft <= xRight);
        int count = xRight - xLeft;
        // GPixel shaded[count];
        if (count <= 0) {
            return;
        }
        std::vector<GPixel> shaded(count);

        shader->shadeRow(xLeft, y, count, shaded.data());

        if (shader->isOpaque()) {
            for (int x = xLeft; x < xRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                GPixel srcPixel = shaded[x - xLeft];

                *addr = srcPixel;
            }
        } else {
            for (int x = xLeft; x < xRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                GPixel srcPixel = shaded[x - xLeft];

                GPixel blendedPixel = blendColors(srcPixel, *addr, blendMode);

                *addr = blendedPixel;
            }
        }
    }
}

// check edges for consistency in sorting both by x and y. could also use exit after both sorts to check
void MyCanvas::pathScan(std::vector<Edge> edges, GShader* shader, GPaint paint, GPixel src) {
    int top = edges.front().y0;
    int left, right;

    while (edges.size() > 0) {
        int i = 0;
        int w = 0;

        while (i < edges.size() && isValidEdge(edges[i], top)) {
            if (w == 0) {
                left = GRoundToInt(edges[i].x0);
            }

            assert(edges[i].wind == 1 || edges[i].wind == -1);

            w += edges[i].wind;

            if (w == 0) {
                right = GRoundToInt(edges[i].x0);
                blit(top, left, right, paint);
            }

            if (!isValidEdge(edges[i], top + 1)) {
                edges.erase(edges.begin() + i);
            } else {
                edges[i].x0 += edges[i].m;
                i++;
            }
        }

        assert(w == 0);
        top++;

        while (i < edges.size() && isValidEdge(edges[i], top)) {
            i++;
        }

        std::sort(edges.begin(), edges.begin() + i, sortEdgesByX);
    }
}

void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
    std::shared_ptr<GPath> copy = path.transform(fCTM);

    std::vector<Edge> edges = pathBuildEdges(copy, fDevice.width(), fDevice.height());

    if (edges.size() < 2) {
        return;
    }

    std::sort(edges.begin(), edges.end(), sortEdges);

    GPixel src = color_to_pixel(paint.getColor());
    GShader* shader = paint.peekShader();

    pathScan(edges, shader, paint, src);
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
    int n = 0;
    std::shared_ptr<GShader> shader = paint.shareShader();

    for (int i = 0; i < count; ++i) {
        const GPoint point0 = verts[indices[n+0]];
        const GPoint point1 = verts[indices[n+1]];
        const GPoint point2 = verts[indices[n+2]];

        const GPoint trianglePts[3] = { point0, point1, point2 };

        if (texs != nullptr && colors != nullptr) {
            const GPoint texPts[3] = { texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]] };
            const GColor triangleColors[3] = { colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]] };
            drawTriangleWithTexColor(trianglePts, triangleColors, texPts, shader);
            // draw color and tex triangle
        } else if (colors != nullptr) {
            const GColor triangleColors[3] = { colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]] };
            // draw color triangle
            std::shared_ptr<GShader> triangle = std::make_shared<TriangleShader>(trianglePts, triangleColors);
            GPaint newPaint = GPaint(triangle);
            drawTriangle(trianglePts, newPaint);
        } else if (texs != nullptr && shader != nullptr) {
            const GPoint texPts[3] = { texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]] };
            drawTriangleWithTex(trianglePts, texPts, shader);
            // draw tex triangle
        }

        // if (colors != nullptr) {
        //     const GColor triangleColors[3] = { colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]] };
        //     // draw color triangle
        //     std::shared_ptr<GShader> triangle = std::make_shared<TriangleShader>(trianglePts, triangleColors);
        //     GPaint newPaint = GPaint(triangle);
        //     drawTriangle(trianglePts, newPaint);

        // } else if (texs != nullptr && shader != nullptr) {
        //     const GPoint texPts[3] = { texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]] };
        //     drawTriangleWithTex(trianglePts, texPts, shader);
        //     // draw tex triangle

        // } else if (texs != nullptr && colors != nullptr) {
        //     const GPoint texPts[3] = { texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]] };
        //     const GColor triangleColors[3] = { colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]] };
        //     drawTriangleWithTexColor(trianglePts, triangleColors, texPts, shader);
        //     // draw color and tex triangle
        // }
        n += 3;
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
    GPoint quadVerts[4];
    GColor quadColors[4];
    GPoint quadTexs[4];

    for (int i = 0; i <= level; i++) {
        float a = (float) i / ((float) (level + 1.0f));
        float b = (float) (i + 1) / ((float) (level + 1.0f));

        for (int j = 0; j <= level; j++) {
            float c = (float) j / ((float) (level + 1.0f));
            float d = (float) (j + 1) / ((float) (level + 1.0f));

            quadVerts[0] = getQuadrilateralPoint(verts, a, c);
            quadVerts[1] = getQuadrilateralPoint(verts, b, c);
            quadVerts[2] = getQuadrilateralPoint(verts, b, d);
            quadVerts[3] = getQuadrilateralPoint(verts, a, d);

            if (colors != nullptr) {
                quadColors[0] = getQuadColor(colors, a, c);
                quadColors[1] = getQuadColor(colors, b, c);
                quadColors[2] = getQuadColor(colors, b, d);
                quadColors[3] = getQuadColor(colors, a, d);
            }

            if (texs != nullptr) {
                quadTexs[0] = getQuadrilateralPoint(texs, a, c);
                quadTexs[1] = getQuadrilateralPoint(texs, b, c);
                quadTexs[2] = getQuadrilateralPoint(texs, b, d);
                quadTexs[3] = getQuadrilateralPoint(texs, a, d);
            }

            const int indices[6] = { 1, 2, 3, 0, 1, 3 };

            if (colors != nullptr && texs != nullptr) {
                drawMesh(quadVerts, quadColors, quadTexs, 2, indices, paint);
            } else if (colors != nullptr) {
                drawMesh(quadVerts, quadColors, nullptr, 2, indices, paint);
            } else if (texs != nullptr) {
                drawMesh(quadVerts, nullptr, quadTexs, 2, indices, paint);
            } else {
                drawMesh(quadVerts, nullptr, nullptr, 2, indices, paint);
            }
        }
    }

}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = gFloatPI * 2 / count;

    for (int i = 0; i < count; ++i) {
        pts[i] = {cx + std::cos(angle) * radius, cy + std::sin(angle) * radius};
        angle += deltaAngle;
    }
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    GPoint storage[12];
    for (int count = 12; count >= 3; --count) {
        make_regular_poly(storage, count, 256, 256, count * 10 + 120);
        for (int i = 0; i < count; ++i) {
            storage[i].x += 2;
            storage[i].y += 3;
        }
        GColor c = GColor::RGBA(std::abs(sinf(count*7)),
                                std::abs(sinf(count*11)),
                                std::abs(sinf(count*17)),
                                0.8f);
        canvas->drawConvexPolygon(storage, count, GPaint(c));
    }
    return "WOOO";
}
