/**
 *  Copyright 2024 Tyler Roth
 */

#ifndef GEdge_DEFINED
#define GEdge_DEFINED

#include "include/GPoint.h"
#include "include/GPath.h"
#include "include/GBitmap.h"
#include "starter_path.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <vector>

struct Edge {
    GPoint start;
    GPoint end;
    float m, b, x0;
    int y0, y1;
    int wind;

    Edge(const GPoint& p1, const GPoint& p2, const int wind) : start(p1), end(p2), wind(wind) {
        if (start.y > end.y) {
            std::swap(start, end);
            this->wind = -this->wind;
        }
        this->wind = wind;
        calculateProperties();
    }

    void calculateProperties() {
        m = (end.x - start.x) / (end.y - start.y);
        b = start.x - m * start.y;
        y0 = GRoundToInt(std::min(start.y, end.y));
        y1 = GRoundToInt(std::max(start.y, end.y));
        x0 = m * (y0 + 0.5f) + b;
    }

    float computeX(int y) const {
        return start.x + m * (y - start.y);
    }
};

inline bool isValidEdge(Edge edge, int y) {
    return edge.y0 <= y && edge.y1 > y;
}

inline std::vector<Edge> clipEdges(int bottom, int right, GPoint p0, GPoint p1) {
    std::vector<Edge> edges;
    int wind;

    if (p0.y == p1.y) {
        return edges;
    }

    if (p0.y > p1.y) {
        std::swap(p0, p1);
        wind = -1;
    } else {
        wind = 1;
    }

    if (p1.y <= 0 || p0.y >= bottom) {
        return edges;
    }

    float m = (p1.x - p0.x) / (p1.y - p0.y);
    float b = p0.x - (p0.y * m);

    if (p0.y < 0) {
        p0.x = b;
        p0.y = 0;
    }

    if (p1.y > bottom) {
        p1.x = m * bottom + b;
        p1.y = bottom;
    }

    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }

    // left projection
    if (p1.x <= 0) {
        Edge edge = {{0, p0.y}, {0, p1.y}, wind};

        // ADD ISVALIDEDGE
        if (isValidEdge(edge, GRoundToInt(edge.y0))) {
            edges.push_back(edge);
        }
        
        return edges;
    }

    // right projection
    if (p0.x >= right) {
        Edge edge = {{right, p0.y}, {right, p1.y}, wind};
    
        if (isValidEdge(edge, GRoundToInt(edge.y0))) {
            edges.push_back(edge);
        }

        return edges;
    }

    // left straddle
    if (p0.x < 0) {
        float newY = (b / m) * -1;

        Edge edge = {{0, p0.y}, {0, newY}, wind};

        p0 = {0, newY};

        if (isValidEdge(edge, GRoundToInt(edge.y0))) {
            edges.push_back(edge);
        }
    }

    // right straddle
    if (p1.x > right) {
        float newY = (right-b) / m;

        Edge edge = {{right, p1.y}, {right, newY}, wind};

        p1 = {static_cast<float>(right), newY};

        if (isValidEdge(edge, GRoundToInt(edge.y0))) {
            edges.push_back(edge);
        }
    }

    Edge lastEdge = {p0, p1, wind};

    if (isValidEdge(lastEdge, GRoundToInt(lastEdge.y0))) {
        edges.push_back(lastEdge);
    }

    return edges;
}

inline int quadSegmentsNum(GPoint points[3]) {
    GPoint error = (points[0] - (2 * points[1]) + points[2]) * 0.25f; // multiply instead of divide
    float distance = sqrt((error.x * error.x) + (error.y * error.y));

    return (int) ceil(sqrt(distance * 4));
}

inline int cubicSegmentsNum(GPoint points[4]) {
    GPoint error1 = points[0] - 2 * points[1] + points[2];
    GPoint error2 = points[1] - 2 * points[2] + points[3];

    float error_x = std::max(abs(error1.x), abs(error2.x));
    float error_y = std::max(abs(error1.y), abs(error2.y));

    float distance = sqrt((error_x * error_x) + (error_y * error_y));

    return (int) ceil(sqrt(distance * 3));
}

inline std::vector<Edge> buildEdges(GBitmap device, int count, const GPoint points[]) {
    int deviceRight = device.width();
    int deviceBottom = device.height();

    std::vector<Edge> edges;

    for (int i = 0; i < count; i++) {
        GPoint pt0 = points[i];
        GPoint pt1 = points[(i + 1) % count];

        std::vector<Edge> clipped = clipEdges(deviceBottom, deviceRight, pt0, pt1);

        if (clipped.size() > 0) {
            edges.insert(edges.end(), clipped.begin(), clipped.end());
        }
    }

    return edges;
}

inline std::vector<Edge> pathBuildEdges(std::shared_ptr<GPath> path, int width, int height) {
    std::vector<Edge> edges;
    GPoint points[GPath::kMaxNextPoints];
    GPath::Edger iterator(*path);
    GPoint currentPoint;

    while (auto verb = iterator.next(points)) {
        int segmentsNum;
        GPoint point1, point2;
        float t;
        std::vector<Edge> new_edges;

        switch (verb.value()) {
            case GPathVerb::kLine:
                new_edges = clipEdges(height, width, points[0], points[1]);

                if (new_edges.size() > 0) {
                    edges.insert(edges.end(), new_edges.begin(), new_edges.end());
                }
                
                currentPoint = points[0];
                break;

            case GPathVerb::kMove:
                // hmmmmm
                currentPoint = points[0];
                break;

            case GPathVerb::kQuad:
                point1 = points[0];
                segmentsNum = quadSegmentsNum(points);

                for (int i = 1; i < segmentsNum; i++) {
                    t = i * (1.0f / segmentsNum);

                    point2 = { getQuadPoint(points[0].x, points[1].x, points[2].x, t).ABC, 
                               getQuadPoint(points[0].y, points[1].y, points[2].y, t).ABC };

                    new_edges = clipEdges(height, width, point1, point2);

                    if (new_edges.size() > 0) {
                        edges.insert(edges.end(), new_edges.begin(), new_edges.end());
                    }

                    point1 = point2;
                }

                new_edges = clipEdges(height, width, point1, points[2]);

                if (new_edges.size() > 0) {
                        edges.insert(edges.end(), new_edges.begin(), new_edges.end());
                }

                break;

            case GPathVerb::kCubic:
                point1 = points[0];
                segmentsNum = cubicSegmentsNum(points);

                for (int i = 1; i < segmentsNum; i++) {
                    t = i * (1.0f / segmentsNum);

                    point2 = { getCubicPoint(points[0].x, points[1].x, points[2].x, points[3].x, t).ABCD, 
                               getCubicPoint(points[0].y, points[1].y, points[2].y, points[3].y, t).ABCD };

                    new_edges = clipEdges(height, width, point1, point2);

                    if (new_edges.size() > 0) {
                        edges.insert(edges.end(), new_edges.begin(), new_edges.end());
                    }

                    point1 = point2;
                }

                new_edges = clipEdges(height, width, point1, points[3]);

                if (new_edges.size() > 0) {
                        edges.insert(edges.end(), new_edges.begin(), new_edges.end());
                }

                break;

        }
    }

    return edges;

}

inline bool sortEdgesByX(Edge e0, Edge e1) {
    return e0.x0 < e1.x0;
}

inline bool sortEdges(Edge e0, Edge e1) {
    if (e0.y0 < e1.y0) {
        return true;
    } else if (e1.y0 < e0.y0) {
        return false; 
    }

    return e0.x0 < e1.x0;
}

#endif