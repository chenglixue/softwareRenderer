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

//normal mapping & TBN
struct NMPhoneShader : public IShader
{
    mat <2, 3, float> m_varyingUV;
    mat <3, 3, float> m_varyingNormal;
    mat <3, 3, float> m_ndc;
    mat <4, 3, float> m_varyingTri;

    virtual Vec4f Vertex(int iFace, int nthVert)
    {
        m_varyingUV.set_col(nthVert, g_model->uv(iFace, nthVert));
        m_varyingNormal.set_col(nthVert, proj<3>((g_Projection * g_ModelView).invert_transpose() * embed<4>(g_model->normal(iFace, nthVert), 0.f)));
        Vec4f m_vertex = g_Projection * g_ModelView * embed<4>(g_model->vert(iFace, nthVert)); // read the vertex from .obj file
        m_varyingTri.set_col(nthVert, m_vertex);
        m_ndc.set_col(nthVert, proj<3>(m_vertex / m_vertex[3]));
        return m_vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor& color)
    {
        Vec2f uv = m_varyingUV * bar;
        Vec3f bn = (m_varyingNormal * bar).normalize();

        //because transformed from world space to tangen space so use inverse of matrix
        mat<3, 3, float> A;
        A[0] = m_ndc.col(1) - m_ndc.col(0);
        A[1] = m_ndc.col(2) - m_ndc.col(0);
        A[2] = bn;
        mat<3,3,float> AI = A.invert();
        auto i = AI * Vec3f(m_varyingUV[0][1] - m_varyingUV[0][0], m_varyingUV[0][2] - m_varyingUV[0][0], 0);
        auto j = AI * Vec3f(m_varyingUV[1][1] - m_varyingUV[1][0], m_varyingUV[1][2] - m_varyingUV[1][0], 0);
        
        //get the texture transformed to tangen space
        mat<3, 3, float>B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        auto n = (B * g_model->normal(uv)).normalize();
        float diff = std::max(0.f, n * g_LightDir);
        color = g_model->diffuse(uv) * diff;
        return false;                              // whether discard the pixel.true : yes, false : no;
    }
};

int main(int argc, char** argv)
{
    if (2 > argc) 
    {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    float* ZBuffer = new float[g_Width * g_Height];
    for (int i = g_Width * g_Height; i--; ZBuffer[i] = -std::numeric_limits<float>::max());

    TGAImage frameBuffer(g_Width, g_Height, TGAImage::RGB); // the output image
    LookAt(g_Camera, g_Target, g_Up);
    Projection( -1.f / (g_Camera - g_Target).norm());
    ViewPort(g_Width / 8, g_Height / 8, g_Width * 3 / 4, g_Height * 3 / 4);
    g_LightDir = proj<3>((g_Projection * g_ModelView * embed<4>(g_LightDir, 0.f))).normalize();

    //vert f x/y/z x/y/z x/y/z : x = vertex coord, y = uv coor, z = normal coor
    //face : triangle face
    for (int m = 1; m < argc; ++m)
    {
        g_model = new Model(argv[m]);
        NMPhoneShader shader;
        for (int i = 0; i < g_model->nfaces(); ++i)
        {
            for (int j : {0, 1, 2})
            {
                shader.Vertex(i, j);
            }
            rasterization(shader.m_varyingTri, shader, frameBuffer, ZBuffer);
        }
        delete g_model;
    }

    //Image.flip_vertically();
    frameBuffer.flip_vertically();
    frameBuffer.write_tga_file("output.tga");

    delete[] ZBuffer;
    return 0;
}