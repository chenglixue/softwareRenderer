#pragma once
#include <limits>
//#include "model.h"
#include "tgaimage.h"
/*
inline bool isInsideTriangle( Vec3f* v, float x, float y );

inline void drawTriangle( Vec3f* v, TGAImage& image, TGAColor color );

inline std::tuple<float, float, float> computeBarycentric2D(float x, float y, Vec3f* v);
*/

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);