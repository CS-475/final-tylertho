/*
 *  Copyright 2024 Tyler Roth
*/

#include "include/GShader.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GPixel.h"
#include "include/GMatrix.h"
#include "include/GPoint.h"
#include "my_utils.h"
#include "stdlib.h"
#include <vector>

class GLinearGradient : public GShader {
public:
    GLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode) 
        : fColors(colors, colors + count), fNumColors(count), fTileMode(tileMode) {

        for (int i = 0; i < count; ++i) {
            fDiffColors.push_back(colors[i+1] - colors[i]);
        }

        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;

        fUnitMatrix = { dx, -dy, p0.x, dy, dx, p0.y };
    }

    bool isOpaque() override {
        for (int i = 0; i < fNumColors; i++) {
            if (fColors[i].a != 1.0) {
                return false;
            }
        }
		return true;    
    };

    bool setContext(const GMatrix& ctm) override {
        auto inverseTemp = (ctm * fUnitMatrix).invert();

        if (inverseTemp) {
            fInverseCTM = *inverseTemp;
        } else {
            return false;
        }
        return true;
    };

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint point = { x + 0.5f, y + 0.5f };
        GPoint dst = fInverseCTM * point;

        if (fNumColors == 1) {
            for (int i = 0; i < count; ++i) {
                row[i] = color_to_pixel(fColors[0]);
            }
        } else {
            for (int i = 0; i < count; ++i) {
                float x;
                switch (fTileMode) {
                    case GTileMode::kClamp:
                        x = linearClamp(dst.x);
                        break;

                    case GTileMode::kMirror:
                        x = linearMirror(dst.x);
                        break;

                    case GTileMode::kRepeat:
                        x = linearRepeat(dst.x);
                        break;
                }

                GColor color;
                // float x = std::max(0.0f, std::min(dst.x, 1.0f)) * (fNumColors - 1);
                int k = GFloorToInt(x);
                float t = x - k;

                if (t == 0) {
                    color = fColors[k];
                } else {
                    color = fColors[k] + (t * fDiffColors[k]);
                }

                row[i] = color_to_pixel(color);

                dst.x += fInverseCTM[0];
            }
        }
    };

private:
    const GBitmap fDevice;
    std::vector<GColor> fDiffColors;
    std::vector<GColor> fColors;
    GMatrix fInverseCTM;
    GMatrix fUnitMatrix;
    int fNumColors;
    GTileMode fTileMode;

    float linearClamp(float px) {
        return std::max(0.0f, std::min(px, 1.0f)) * (fNumColors - 1);
    }

    float linearMirror(float px) {
        int p = GFloorToInt(abs(px));

        if ((p % 2) == 0) {
          return (px - GFloorToInt(px)) * (fNumColors-1);
        } else {
          return (GCeilToInt(px) - px) * (fNumColors-1);
        }
    }

    float linearRepeat(float px) {
        return (px - GFloorToInt(px)) * (fNumColors-1);
    }
};

std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode) {
  if (count < 1) {
    return nullptr;
  }

  return std::shared_ptr<GShader>(new GLinearGradient(p0, p1, colors, count, mode));
}