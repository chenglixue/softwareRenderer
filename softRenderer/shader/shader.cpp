#include <cmath>
#include <limits>
#include <cstdlib>
#include "shader.h"

IShader::~IShader(){}

/*
Bresenham's line
void Line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool steep = false; //Whether swap coordinates x and Y
    //When the slope is bigger than 1, swap coordinates x , y.Otherwise there will be holes in the line segment
    if (std::abs(y0 - y1) > std::abs(x0 - x1))
    {

        std::swap(x0, y0);
        std::swap(x1, y1);

        steep = true;
    }

    //When the X-coordinate of the starting point is bigger than the X-coordinate of the ending point,swap them.Otherwise, can't draw a line
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    //Algorithm optimization.use int instead of float 
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror = std::abs(2 * dy);
    int error = 0;

    for (int x = x0, y = y0; x <= x1; ++x)
    {
        //Whether  swap coordinates x and Y
        if (steep == true)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }

        //Bresenham's line
        error += derror;
        if (error > 0)
        {

            y += y0 > y1 ? -1 : 1;
            error -= 2 * dx;

        }
    }
}
*/

/*
void DrawTriangle(vec2* v, TGAImage& image, const TGAColor& color)
{
    if (v[0].y > v[1].y) std::swap(v[0], v[1]);
    if (v[0].y > v[2].y) std::swap(v[0], v[2]);
    if (v[1].y > v[2].y) std::swap(v[1], v[2]);

    int totalHeight = v[2].y - v[0].y + 1;

    for (int _y = v[0].y; _y <= v[1].y; ++_y)
    {
        int target = v[1].y - v[0].y + 1;
        float alpha = (float)(_y - v[0].y) / totalHeight;
        float beta = (float)(_y - v[0].y) / target;

        vec2 A = v[0] + (v[2] - v[0]) * alpha;
        vec2 B = v[0] + (v[1] - v[0]) * beta;

        if (A.x > B.x) std::swap(A, B);

        for (int i = A.x; i <= B.x; ++i)
            image.set(i, _y, color);
    }

    for (int _y = v[1].y; _y <= v[2].y; ++_y)
    {
        int target = v[2].y - v[1].y + 1;
        float alpha = (float)(_y - v[0].y) / totalHeight;
        float beta = (float)(_y - v[1].y) / target;

        vec2 A = v[0] + (v[2] - v[0]) * alpha;
        vec2 B = v[1] + (v[2] - v[1]) * beta;

        if (A.x > B.x) std::swap(A, B);

        for (int i = A.x; i <= B.x; ++i)
            image.set(i, _y, color);
    }
}
*/ 

// interpolation of triangle
Vec3f ComputeBarycentric(Vec2f P, Vec2f A, Vec2f B, Vec2f C)
{
    float c1 = (P.x * (B.y - C.y) + (C.x - B.x) * P.y + B.x * C.y - C.x * B.y) / (A.x * (B.y - C.y) + (C.x - B.x) * A.y + B.x * C.y - C.x * B.y);
    float c2 = (P.x * (C.y - A.y) + (A.x - C.x) * P.y + C.x * A.y - A.x * C.y) / (B.x * (C.y - A.y) + (A.x - C.x) * B.y + C.x * A.y - A.x * C.y);
    float c3 = (P.x * (A.y - B.y) + (B.x - A.x) * P.y + A.x * B.y - B.x * A.y) / (C.x * (A.y - B.y) + (B.x - A.x) * C.y + A.x * B.y - B.x * A.y);
    return Vec3f(c1, c2, c3);
}

/*
 for tangent space
void Rasterization(mat<4, 3, float>& clipc, IShader& shader, TGAImage& image, float* zbuffer)
{
    mat<3, 4, float> pts = (g_Viewport * clipc).transpose(); // transposed to ease access to each of the points
    mat<3, 2, float> pts2;
    for (int i = 0; i < 3; i++) pts2[i] = proj<2>(pts[i] / pts[i][3]);

    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 2; j++) 
        {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
        }
    }

    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) 
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) 
        {
            Vec3f weight = ComputeBarycentric(P, pts2[0], pts2[1], pts2[2]);
            if (weight.x < 0 || weight.y < 0 || weight.z < 0) continue;

            //normalize weight value
            Vec3f clipInter = Vec3f(weight.x / pts[0][3], weight.y / pts[1][3], weight.z / pts[2][3]);
            clipInter = clipInter / (clipInter.x + clipInter.y + clipInter.z);
            float zInter = clipc[2] * clipInter;

            if (zbuffer[P.x + P.y * image.get_width()] > zInter) continue;

            bool discard = shader.fragment(clipInter, color);
            if (!discard) 
            {
                zbuffer[P.x + P.y * image.get_width()] = zInter;
                image.set(P.x, P.y, color);
            }
        }
    }
}
*/

void Rasterization(Vec4f* pts, IShader& shader, TGAImage& image, float* zbuffer)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }

    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f weight = ComputeBarycentric(P, proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]));
            if (weight.x < 0 || weight.y < 0 || weight.z < 0) continue;

            float zInter = pts[0][2] * weight.x + pts[1][2] * weight.y + pts[2][2] * weight.z;
            float wInter = pts[0][3] * weight.x + pts[1][3] * weight.y + pts[2][3] * weight.z;
            int fragZ = zInter / wInter;

            if (zbuffer[P.x + P.y * image.get_width()] > zInter) continue;

            bool discard = shader.fragment(weight, color);
            if (!discard)
            {
                zbuffer[P.x + P.y * image.get_width()] = zInter;
                image.set(P.x, P.y, color);
            }
        }
    }
}