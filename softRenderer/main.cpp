#include <vector>
#include <iostream>

#include "./.h/model.h"
#include "./shader/shader.h"
#include "./.h/geometry.h"
#include "./.h/tgaimage.h"

//height and width of framebuffer
constexpr int g_Width = 800;
constexpr int g_Height = 800;

Model* g_Model = nullptr;   //.obj model
float* g_ZBuffer = nullptr; //z buffer
float* g_ShadowBuffer = nullptr;    // light z buffer

Vec3f g_LightDir { -1, 1, 1 };  // light
Vec3f g_Camera{ 2, 1, 3 };  // camera
Vec3f g_Target{0, 0, 0};    // target point
Vec3f g_Up{ 0,1,0 };    // unit up direction in world space
Matrix g_ModelView;     // model and view transform matrix
Matrix g_Projection;    // perspective projection transform matrix
Matrix g_Viewport;      // viewport transform matrix

// normal map
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
        m_varyingUV.set_col(nthVert, g_Model->uv(iFace, nthVert));
        //m_varyingIntensity[nthVert] = std::max(0.f, g_model->normal(iFace, nthVert) * g_LightDir); // get diffuse lighting intensity
        Vec4f m_vertex = embed<4>(g_Model->vert(iFace, nthVert)); // read the vertex from .obj file
        return g_Viewport * g_Projection * g_ModelView * m_vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) 
    {
        Vec2f uv = m_varyingUV * bar;
        auto n = proj<3>(m_uniformMIT * embed<4>(g_Model->normal(uv))).normalize();
        auto l = proj<3>(m_uniformM * embed<4>(g_LightDir)).normalize();
        auto intensity = std::max(0.f, n * l);
        color = g_Model->diffuse(uv) * intensity;
        return false;                              // whether discard the pixel.true : yes, false : no;
    }
};

// phone light
struct PhoneShader : public IShader
{
    mat < 2, 3, float > m_varyingUV;
    mat<4, 4, float> m_uniformM;
    mat<4, 4, float> m_uniformMIT;

    virtual Vec4f Vertex(int iFace, int nthVert)
    {
        m_varyingUV.set_col(nthVert, g_Model->uv(iFace, nthVert));
        Vec4f m_vertex = embed<4>(g_Model->vert(iFace, nthVert)); // read the vertex from .obj file
        return g_Viewport * g_Projection * g_ModelView * m_vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor& color)
    {
        Vec2f uv = m_varyingUV * bar;
        auto n = proj<3>(m_uniformMIT * embed<4>(g_Model->normal(uv))).normalize();
        auto l = proj<3>(m_uniformM * embed<4>(g_LightDir)).normalize();
        auto r = (n * (n * l) * 2 - l).normalize();
        float spec = std::pow(std::max(r.z, 0.f), g_Model->specular(uv));
        float diff = std::max(0.f, n * l);
        TGAColor c = g_Model->diffuse(uv);
        color = c;
        for (int i = 0; i < 3; ++i)
            color[i] = std::min<float>(5 + c[i] * (diff + spec * 0.6), 255);
        return false;                              // whether discard the pixel.true : yes, false : no;
    }
};

//normal mapping & TBN & phone light
struct NMPhoneShader : public IShader
{
    mat <2, 3, float> m_varyingUV;
    mat <3, 3, float> m_varyingNormal;
    mat <3, 3, float> m_ndc;
    mat <4, 3, float> m_varyingTri;

    virtual Vec4f Vertex(int iFace, int nthVert)
    {
        m_varyingUV.set_col(nthVert, g_Model->uv(iFace, nthVert));
        m_varyingNormal.set_col(nthVert, proj<3>((g_Projection * g_ModelView).invert_transpose() * embed<4>(g_Model->normal(iFace, nthVert), 0.f)));
        Vec4f m_vertex = g_Projection * g_ModelView * embed<4>(g_Model->vert(iFace, nthVert)); // read the vertex from .obj file
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

        auto n = (B * g_Model->normal(uv)).normalize();
        float diff = std::max(0.f, n * g_LightDir);
        color = g_Model->diffuse(uv) * diff;
        return false;                              // whether discard the pixel.true : yes, false : no;
    }
};

// get light buffer in 1st PASS in shadow map
struct DepthShader : public IShader
{
    mat <3, 3, float> m_varyingTri;

    //DepthShader() : m_varyingTri() {}

    virtual Vec4f Vertex(int nFace, int nthVert)
    {
        Vec4f vertex = embed<4>(g_Model->vert(nFace, nthVert));
        vertex = g_Viewport * g_Projection * g_ModelView * vertex;
        m_varyingTri.set_col(nthVert, proj<3>(vertex / vertex[3]));
        return vertex;
    }

    virtual bool fragment(Vec3f weight, TGAColor& color)
    {
        Vec3f pInter = m_varyingTri * weight;
        color = TGAColor(255, 255, 255) * (pInter.z / depth);
        return false;
    }
};

// get framebuffer in 2nd PASS in shadow map
struct ShadowMapShader : public IShader
{
    mat <3, 3, float> m_varyingTri;
    mat <2, 3, float> m_varyingUV;
    mat <4, 4, float> m_uniformM;
    mat <4, 4, float> m_uniformMIT;
    mat <4, 4, float> m_uniformMShadow; // transform camera's viewport into light's

    ShadowMapShader(Matrix m, Matrix mIT, Matrix mS) : 
        m_uniformM(m), m_uniformMIT(mIT), m_uniformMShadow(mS), m_varyingTri(), m_varyingUV() {}

    virtual Vec4f Vertex(int nFace, int nthVert)
    {
        m_varyingUV.set_col(nthVert, g_Model->uv(nFace, nthVert));
        auto vertex = embed<4>(g_Model->vert(nFace, nthVert));
        vertex = g_Viewport * g_Projection * g_ModelView * vertex;
        m_varyingTri.set_col(nthVert, proj<3>(vertex / vertex[3]));
        return vertex;
    }

    virtual bool fragment(Vec3f weight, TGAColor& color)
    {
        Vec4f shadBuffP = m_uniformMShadow * embed<4>(m_varyingTri * weight);
        shadBuffP = shadBuffP / shadBuffP[3];
        
        int index = static_cast<int>(shadBuffP[0]) + static_cast<int>(shadBuffP[1]) * g_Width;
        //std::cout << index << std::endl;
        double shadow = 0.3 + (g_ShadowBuffer[index] < shadBuffP[2]) * .7;

        auto uv = m_varyingUV * weight;
        auto n = proj<3>(m_uniformMIT * embed<4>(g_Model->normal(uv))).normalize();
        auto l = proj<3>(m_uniformM * embed<4>(g_LightDir));
        auto r = (n * (n * l) * 2 - l).normalize();
        float spec = std::pow(std::max(r.z, 0.f), g_Model->specular(uv));
        float diff = std::max(0.f, n * l);
        TGAColor c = g_Model->diffuse(uv);
        for (int i : {0, 1, 2})
            color[i] = std::min<float>(20 + c[i] * shadow * (1.2 * diff + 0.6 * spec), 255);
        return false;
    }
};

int main(int argc, char** argv)
{
    if (2 > argc) 
    {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    //initialize z buffer
    g_ZBuffer = new float[g_Width * g_Height];
    g_ShadowBuffer = new float[g_Width * g_Height];
    for (int i = g_Width * g_Height; --i; )
        g_ZBuffer[i] = g_ShadowBuffer[i] = std::numeric_limits<float>::min();

    g_LightDir.normalize();

    // output the zbuffer of point light
    {
        TGAImage depth(g_Width, g_Height, TGAImage::RGB); 

        LookAt(g_Camera, g_Target, g_Up);
        Projection(0);
        ViewPort(g_Width / 8, g_Height / 8, g_Width * 3 / 4, g_Height * 3 / 4);

        for (int m = 1; m < argc; ++m)
        {
            g_Model = new Model(argv[m]);   //get .obj model
            DepthShader shader; //initialize shader
            Vec4f screen_coords[3];
            for (int i = 0; i < g_Model->nfaces(); ++i) // triangle face
            {
                for (int j : {0, 1, 2}) //vertex of cur triangle
                {
                    screen_coords[j] = shader.Vertex(i, j); //viewport coordinates
                }
                Rasterization(screen_coords, shader, depth, g_ShadowBuffer);    //get framebuffer
            }
            delete g_Model;
        }

        depth.flip_vertically();
        depth.write_tga_file("depth.tga");
    }

    //this matrix is used to transform camera's framebuffer into light's
    auto lightM = g_Viewport * g_Projection * g_ModelView;

    //output result
    {
        TGAImage frameBuffer(g_Width, g_Height, TGAImage::RGB); // the output image
        LookAt(g_Camera, g_Target, g_Up);   //view transform
        Projection(-1.f / (g_Camera - g_Target).norm());    //perspective projection transform
        ViewPort(g_Width / 8, g_Height / 8, g_Width * 3 / 4, g_Height * 3 / 4); //viewport transform

        //vert f x/y/z x/y/z x/y/z : x = vertex coord, y = uv coor, z = normal coor
        //face : triangle face
        for (int m = 1; m < argc; ++m)
        {
            g_Model = new Model(argv[m]);
            ShadowMapShader shader(g_ModelView, g_Projection * g_ModelView.invert_transpose(), lightM * (g_Viewport * g_Projection * g_ModelView).invert());
            Vec4f screen_coords[3];
            for (int i = 0; i < g_Model->nfaces(); ++i)
            {
                for (int j : {0, 1, 2})
                {
                    screen_coords[j] = shader.Vertex(i, j); //viewport coordinates
                }
                Rasterization(screen_coords, shader, frameBuffer, g_ZBuffer);
            }
            delete g_Model;
        }

        //Image.flip_vertically();
        frameBuffer.flip_vertically();
        frameBuffer.write_tga_file("output.tga");
    }

    delete[] g_ZBuffer;
    delete[] g_ShadowBuffer;
    return 0;
}