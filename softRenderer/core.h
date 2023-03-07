#pragma once
#include <limits>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

void Line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

void DrawTriangle(Vec2i* v, TGAImage& image, const TGAColor& color);

void DrawTriangleZBuffer( Vec3f* v, std::vector<std::vector<float>>&, int width, TGAImage& image, TGAColor color );

inline Vec3f ComputeBarycentric(Vec3f* v, float x, float y)
{
    float c1 = (x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * y + v[1].x * v[2].y - v[2].x*v[1].y) / (v[0].x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * v[0].y + v[1].x * v[2].y - v[2].x * v[1].y );
    float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x*v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y );
    float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x*v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y );
    return Vec3f(c1, c2, c3);
}

inline bool IsInsideTriangle(Vec3f* v, int x, int y)
{
    Vec2f side1 = { v[1].x - v[0].x, v[1].y - v[0].y };
    Vec2f side2 = { v[2].x - v[1].x, v[2].y - v[1].y };
    Vec2f side3 = { v[0].x - v[2].x, v[0].y - v[2].y };

    Vec2f v1 = { x - v[0].x, y - v[0].y };
    Vec2f v2 = { x - v[1].x, y - v[1].y };
    Vec2f v3 = { x - v[2].x, y - v[2].y };

    float z1 = side1.x * v1.y - v1.x * side1.y;
    float z2 = side2.x * v2.y - v2.x * side2.y;
    float z3 = side3.x * v3.y - v3.x * side3.y;

    if( ( z1 > 0 && z2 > 0 && z3 > 0) || ( z1 < 0 && z2 < 0 && z3 < 0 ) ) return true;
    return false;
}