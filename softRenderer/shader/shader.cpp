#include <cmath>
#include <limits>
#include <cstdlib>
#include "shader.h"

IShader::~IShader(){}

/*
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

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) 
{
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void rasterization(mat<4, 3, float>& clipc, IShader& shader, TGAImage& image, float* zbuffer)
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
            Vec3f bc_screen = barycentric(pts2[0], pts2[1], pts2[2], P);
            Vec3f bc_clip = Vec3f(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
            float frag_depth = clipc[2] * bc_clip;
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z<0 || zbuffer[P.x + P.y * image.get_width()] > frag_depth) continue;
            bool discard = shader.fragment(bc_clip, color);
            if (!discard) 
            {
                zbuffer[P.x + P.y * image.get_width()] = frag_depth;
                image.set(P.x, P.y, color);
            }
        }
    }
}