/*
 *  Copyright 2024 Tyler Roth
*/

#include "include/GMatrix.h"
#include "include/GPath.h"
#include "include/GPoint.h"
#include "include/GRect.h"
#include "include/GPathBuilder.h"
#include "include/GPath.h"
#include "starter_path.h"
#include <iostream>
#include <initializer_list> 

#define dT_constant 0.5519150244935105707435627f;

QuadraticCurve getQuadPoint(float A, float B, float C, float t) {
    float AB = A + t * (B - A);
    float BC = B + t * (C - B);
    float ABC = AB + t * (BC - AB);

    return { AB, BC, ABC };
}

CubicCurve getCubicPoint(float A, float B, float C, float D, float t) {
    QuadraticCurve ABC = getQuadPoint(A, B, C, t);
    QuadraticCurve BCD = getQuadPoint(B, C, D, t);

    float ABCD = ABC.ABC + (t * (BCD.ABC - ABC.ABC));

    return { ABC, BCD, ABCD };
}

void GPathBuilder::addRect(const GRect& r, GPathDirection dir) {
    this->moveTo({r.left, r.top});

    if (dir == GPathDirection::kCW) {
        this->lineTo({r.right, r.top});
        this->lineTo({r.right, r.bottom});
        this->lineTo({r.left, r.bottom});
    } else {
        this->lineTo({r.left, r.bottom});
        this->lineTo({r.right, r.bottom});
        this->lineTo({r.right, r.top});
    }
}

void GPathBuilder::addPolygon(const GPoint pts[], int count) {
    assert(count >= 2);

    this->moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        this->lineTo(pts[i]);
    }
}

GRect GPath::bounds() const {  
    int count = this->fPts.size();
    
    if (count == 0) {
        return GRect{0.0, 0.0, 0.0, 0.0};
    }

    if (count == 1) {
        return GRect{fPts[0].x, fPts[0].y, 0.0, 0.0};
    }

    float min_x = fPts[0].x;
    float max_x = fPts[0].x;
    float min_y = fPts[0].y;
    float max_y = fPts[0].y;

    GPoint points[GPath::kMaxNextPoints];
    GPath::Edger iterator(*this);

    while (auto verb = iterator.next(points)) {
        float ax, bx, cx, ay, by, cy;
        float t1, t2;
        GPoint A, B, C, D;

        float point_x1 = fPts[0].x;
        float point_y1 = fPts[0].x;
        float point_x2 = fPts[0].y;
        float point_y2 = fPts[0].y;

        switch (verb.value()) {
            case GPathVerb::kLine:
                min_x = std::min({min_x, points[0].x, points[1].x});
                max_x = std::max({max_x, points[0].x, points[1].x});
                min_y = std::min({min_y, points[0].y, points[1].y});
                max_y = std::max({max_y, points[0].y, points[1].y});

                break;

            case GPathVerb::kMove:
                min_x = std::min(min_x, points[0].x);
                max_x = std::max(max_x, points[0].x);
                min_y = std::min(min_y, points[0].y);
                max_y = std::max(max_y, points[0].y);

                break;

            case GPathVerb::kQuad:
                A = points[0];
                B = points[1];
                C = points[2];

                t1 = (A.x - B.x) / (A.x - 2 * B.x + C.x);
                t2 = (A.y - B.y) / (A.y - 2 * B.y + C.y);

                if(t1 >= 0 && t1 <= 1) {
                    point_x1 = getQuadPoint(A.x, B.x, C.x, t1).ABC;
                    point_y1 = getQuadPoint(A.y, B.y, C.y, t1).ABC;
                }

                if (t2 >= 0 && t2 <= 1) {
                    point_x2 = getQuadPoint(A.x, B.x, C.x, t2).ABC;
                    point_y2 = getQuadPoint(A.y, B.y, C.y, t2).ABC;
                }

                min_x = std::min({min_x, point_x1, point_x2, A.x, C.x});
                max_x = std::max({max_x, point_x1, point_x2, A.x, C.x});
                min_y = std::min({min_y, point_y1, point_y2, A.y, C.y});
                max_y = std::max({max_y, point_y1, point_y2, A.y, C.y});

                break;
                
            case GPathVerb::kCubic:
                A = points[0];
                B = points[1];
                C = points[2];
                D - points[3];

                ax = -A.x + 3 * B.x - 3 * C.x + D.x;
                bx = 2 * A.x - 4 * B.x + 2 * C.x;
                cx = -A.x + B.x;

                ay = -A.y + 3 * B.y - 3 * C.y + D.y;
                by = 2 * A.y - 4 * B.y + 2 * C.y;
                cy = -A.y + B.y;

                if (ax == 0) {
                    t1 = -cx / bx;
                    t2 = -1;
                } else {
                    t1 = (-bx + sqrt(bx * bx - (4 * ax * cx))) / (2 * ax);
                    t2 = (-bx - sqrt(bx * bx - (4 * ax * cx))) / (2 * ax);
                }

                if (t1 >= 0 && t1 <= 1) {
                    point_x1 = getCubicPoint(A.x, B.x, C.x, D.x, t1).ABCD;
                    point_y1 = getCubicPoint(A.y, B.y, C.y, D.y, t1).ABCD;
                }

                if (t2 >= 0 && t2 <= 1) {
                    point_x2 = getCubicPoint(A.x, B.x, C.x, D.x, t2).ABCD;
                    point_y2 = getCubicPoint(A.y, B.y, C.y, D.y, t2).ABCD;
                }

                min_x = std::min({point_x1, point_x2, A.x, D.x, min_x});
                max_x = std::max({point_x1, point_x2, A.x, D.x, max_x});
                min_y = std::min({point_y1, point_y2, A.y, D.y, min_y});
                max_y = std::min({point_y1, point_y2, A.y, D.y, max_y});

                if (ay == 0) {
                    t1 = -cy / by;
                    t2 = -1;
                } else {
                    t1 = (-by + sqrt(by * by - (4 * ay * cy))) / (2 * ay);
                    t2 = (-by - sqrt(by * by - (4 * ay * cy))) / (2 * ay);
                }

                if (t1 >= 0 && t1 <= 1) {
                    point_x1 = getCubicPoint(A.x, B.x, C.x, D.x, t1).ABCD;
                    point_y1 = getCubicPoint(A.y, B.y, C.y, D.y, t1).ABCD;
                }

                if (t2 >= 0 && t2 <= 1) {
                    point_x2 = getCubicPoint(A.x, B.x, C.x, D.x, t2).ABCD;
                    point_y2 = getCubicPoint(A.y, B.y, C.y, D.y, t2).ABCD;
                }

                min_x = std::min({point_x1, point_x2, A.x, D.x, min_x});
                max_x = std::max({point_x1, point_x2, A.x, D.x, max_x});
                min_y = std::min({point_y1, point_y2, A.y, D.y, min_y});
                max_y = std::min({point_y1, point_y2, A.y, D.y, max_y});

                break;
        }
    }

    return GRect{min_x, min_y, max_x, max_y};
}

inline void GPathBuilder::transform(const GMatrix& matrix) {
    matrix.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size());
}

void GPathBuilder::addCircle(GPoint center, float radius, GPathDirection dir) {
    float x = center.x;
    float y = center.y;

    float dist = radius * dT_constant;

    this->moveTo(x + radius, y);

    if (dir == GPathDirection::kCW) {
        this->cubicTo({ x + radius, y + dist }, { x + dist, y + radius }, { x, y + radius }); // bottom 
        this->cubicTo({ x - dist, y + radius }, { x - radius, y + dist }, { x - radius, y }); // left
        this->cubicTo({ x - radius, y - dist }, { x - dist, y - radius }, { x, y - radius}); // top
        this->cubicTo({ x + dist, y - radius }, { x + radius, y - dist }, { x + radius, y }); // right
    } else {
        this->cubicTo({ x + radius, y - dist }, { x + dist, y - radius }, { x, y - radius}); // top
        this->cubicTo({ x - dist, y - radius }, { x - radius, y - dist }, { x - radius, y }); // left
        this->cubicTo({ x - radius, y + dist }, { x - dist, y + radius }, { x, y + radius }); // bottom
        this->cubicTo({ x + dist, y + radius }, { x + radius, y + dist }, { x + radius, y }); // right
    }
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    GPoint A = src[0];
    GPoint B = src[1];
    GPoint C = src[2];

    QuadraticCurve xABC = getQuadPoint(A.x, B.x, C.x, t);
    QuadraticCurve yABC = getQuadPoint(A.y, B.y, C.y, t);

    dst[0] = A;
    dst[1] = { xABC.AB, yABC.AB };
    dst[2] = { xABC.ABC, yABC.ABC };
    dst[3] = { xABC.BC, yABC.BC };
    dst[4] = C;
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    GPoint A = src[0];
    GPoint B = src[1];
    GPoint C = src[2];
    GPoint D = src[3];

    CubicCurve xABCD = getCubicPoint(A.x, B.x, C.x, D.x, t);
    CubicCurve yABCD = getCubicPoint(A.y, B.y, C.y, D.y, t);

    dst[0] = A;
    dst[1] = { xABCD.ABC.AB, yABCD.ABC.AB };
    dst[2] = { xABCD.ABC.ABC, yABCD.ABC.ABC };
    dst[3] = { xABCD.ABCD, yABCD.ABCD };
    dst[4] = { xABCD.BCD.ABC, yABCD.BCD.ABC };
    dst[5] = { xABCD.BCD.BC, yABCD.BCD.BC };
    dst[6] = D;
}
