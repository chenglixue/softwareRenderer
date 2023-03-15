#pragma once
#include "../.h/tgaimage.h"
#include "../.h/geometry.h"

//mvp translate
extern Matrix g_ModelView;
extern Matrix g_Viewport;
extern Matrix g_Projection;
const float depth = 500.f;

struct IShader
{
    virtual ~IShader();
    virtual Vec4f Vertex(int iFace, int nthVert) = 0;   //vertex shader
    virtual bool fragment(const Vec3f bar, TGAColor& color) = 0;    //fragment shader
};

//void Line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

//void DrawTriangle(vec2* v, TGAImage& image, const TGAColor& color);

Vec3f ComputeBarycentric(Vec2f P, Vec2f A, Vec2f B, Vec2f C);

//first target : transform coordinates of vertices
//second target : prepare data for fragment shader
//void Rasterization(mat<4, 3, float>& clipc, IShader& shader, TGAImage& image, float* zbuffer);
void Rasterization(Vec4f* clipc, IShader& shader, TGAImage& image, float* zbuffer);

//always put the object  looking at at (0,0,0)
inline void LookAt(const Vec3f& eyePos, const Vec3f& targetPos, const Vec3f& upDir)
{
    Vec3f z = (eyePos - targetPos).normalize();
    Vec3f x = cross(upDir, z).normalize();
    Vec3f y = cross(z, x).normalize();
    Matrix rotation = Matrix::identity();
    Matrix translation = Matrix::identity();

    for (int i = 0; i < 3; i++) {
        rotation[i][3] = -targetPos[i];
    }
    for (int i = 0; i < 3; i++) {
        rotation[0][i] = x[i];
        rotation[1][i] = y[i];
        rotation[2][i] = z[i];
    }

    g_ModelView = rotation * translation;
}

//perspective projection
inline void Projection(const double& f)
{
    g_Projection = Matrix::identity();
    g_Projection[3][2] = f;
}

//viewport
inline void ViewPort(const int& x, const int& y, const int& w, const int& h)
{
    g_Viewport = Matrix::identity();
    g_Viewport[0][3] = x + w / 2.f;
    g_Viewport[1][3] = y + h / 2.f;
    g_Viewport[2][3] = 255.f / 2.f;

    g_Viewport[0][0] = w / 2.f;
    g_Viewport[1][1] = h / 2.f;
    g_Viewport[2][2] = 255.f / 2.f;
}