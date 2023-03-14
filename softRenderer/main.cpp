#include <vector>
#include <iostream>

#include "./.h/model.h"
#include "./shader/shader.h"
#include "./.h/geometry.h"
#include "./.h/tgaimage.h"

//the resolution of output of the picture
constexpr int g_Width = 800;
constexpr int g_Height = 800;

Model* g_model = nullptr;

Vec3f g_LightDir { 0, -1, 1 };
Vec3f g_Camera{ 2, 1, 3 };
Vec3f g_Target{0, 0, 0};
Vec3f g_Up{ 0,1,0 };
Matrix g_ModelView; 
Matrix g_Projection;
Matrix g_Viewport;

struct NormalMappingShader : public IShader
{
    //const Model& m_model;
    Vec3f m_varyingIntensity; // written by vertex shader, read by fragment shader
    mat < 2, 3, float > m_varyingUV;
    mat<4, 4, float> m_uniformM;
    mat<4, 4, float> m_uniformMIT;

    /*
    GouraudShader(const Model& m) : m_model(m) 
    {
        uniformL = proj<3>((g_ModelView * embed<4>(g_LightDir, 0.))).normalized(); // transform the light vector to view coordinates
    }
    */

    virtual Vec4f Vertex(int iFace, int nthVert) 
    {
        m_varyingUV.set_col(nthVert, g_model->uv(iFace, nthVert));
        //m_varyingIntensity[nthVert] = std::max(0.f, g_model->normal(iFace, nthVert) * g_LightDir); // get diffuse lighting intensity
        Vec4f m_vertex = embed<4>(g_model->vert(iFace, nthVert)); // read the vertex from .obj file
        return g_Viewport * g_Projection * g_ModelView * m_vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) 
    {
        Vec2f uv = m_varyingUV * bar;
        auto n = proj<3>(m_uniformMIT * embed<4>(g_model->normal(uv))).normalize();
        auto l = proj<3>(m_uniformM * embed<4>(g_LightDir)).normalize();
        auto intensity = std::max(0.f, n * l);
        color = g_model->diffuse(uv) * intensity;
        return false;                              // whether discard the pixel.true : yes, false : no;
    }
};

struct PhoneShader : public IShader
{
    mat < 2, 3, float > m_varyingUV;
    mat<4, 4, float> m_uniformM;
    mat<4, 4, float> m_uniformMIT;

    virtual Vec4f Vertex(int iFace, int nthVert)
    {
        m_varyingUV.set_col(nthVert, g_model->uv(iFace, nthVert));
        Vec4f m_vertex = embed<4>(g_model->vert(iFace, nthVert)); // read the vertex from .obj file
        return g_Viewport * g_Projection * g_ModelView * m_vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor& color)
    {
        Vec2f uv = m_varyingUV * bar;
        auto n = proj<3>(m_uniformMIT * embed<4>(g_model->normal(uv))).normalize();
        auto l = proj<3>(m_uniformM * embed<4>(g_LightDir)).normalize();
        auto r = (n * (n * l) * 2 - l).normalize();
        float spec = std::pow(std::max(r.z, 0.f), g_model->specular(uv));
        float diff = std::max(0.f, n * l);
        TGAColor c = g_model->diffuse(uv);
        color = c;
        for (int i = 0; i < 3; ++i)
            color[i] = std::min<float>(5 + c[i] * (diff + spec * 0.6), 255);
        return false;                              // whether discard the pixel.true : yes, false : no;
    }
};

int main(int argc, char** argv)
{
    if (2 == argc) {
        g_model = new Model(argv[1]);
    }
    else {
        g_model = new Model("obj/african_head.obj");
    }

    TGAImage ZBuffer(g_Width, g_Height, TGAImage::GRAYSCALE);
    TGAImage frameBuffer(g_Width, g_Height, TGAImage::RGB); // the output image
    g_LightDir.normalize();

    LookAt(g_Camera, g_Target, g_Up);
    Projection( -1.f / (g_Camera - g_Target).norm());
    ViewPort(g_Width / 8, g_Height / 8, g_Width * 3 / 4, g_Height * 3 / 4);

    //vert f x/y/z x/y/z x/y/z : x = vertex coord, y = uv coor, z = normal coor
    //face : triangle face
    PhoneShader shader;
    shader.m_uniformM = g_Projection * g_ModelView;
    shader.m_uniformMIT = (g_Projection * g_ModelView).invert_transpose();
    for (int i = 0; i < g_model->nfaces(); ++i)
    {
        Vec4f clipVerts[3];  //clip coordinates.written by VS, read by FS
        for (int j : {0, 1, 2})
        {
            clipVerts[j] = shader.Vertex(i, j);
        }
        rasterization(clipVerts, shader, frameBuffer, ZBuffer);
    }

    //Image.flip_vertically();
    frameBuffer.flip_vertically();
    frameBuffer.write_tga_file("output.tga");
    ZBuffer.flip_vertically();
    ZBuffer.write_tga_file("zbuffer.tga");

    delete g_model;
    return 0;
}