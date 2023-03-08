#include "core.h"

const TGAColor White = TGAColor(255, 255, 255, 255);
const TGAColor Red = TGAColor(255, 0, 0, 255);
const TGAColor RandColor = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);

//the resolution of output of the picture
const int Width = 1920;
const int Height = 1080;
const int Depth = 255;
TGAImage Image(Width, Height, TGAImage::RGB);
std::vector<std::vector<int>> ZBuffer(Height + 1, std::vector<int>(Width + 1, std::numeric_limits<int>::min()));
Vec3f LightDir(0, 0, -1);
Vec3f camera(0, 0, 3);

//model data
Model* ModelBody = nullptr;
Model* ModelFace = nullptr;
Model* ModelHair = nullptr;
Model* ModelCloak = nullptr;

int main(int argc, char** argv)
{
    if (2 == argc)
    {
        ModelBody = new Model(argv[1]);
        ModelFace = new Model(argv[1]);
    }
    else
    {
        ModelBody = new Model("obj/fuhuabody.obj");
        ModelFace = new Model("obj/fuhuaface.obj");
        ModelHair = new Model("obj/fuhuahair.obj");
        ModelCloak = new Model("obj/fuhuacloak.obj");
    }
    
    Matrix project = Matrix::identity(4);
    Matrix viewPort = ViewPort(Width / 8, Height / 4, Width * 3 / 4, Height * 3 / 4, Depth);
    project[3][2] = -1.f / camera.z;

    //vert f x/y/z x/y/z x/y/z : x = vertex coord, y = uv coor, z = normal coor
    //face : triangle face
    for (int i = 0; i < ModelBody->nfaces(); ++i)
    {
        std::vector<int> face = ModelBody->face(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];

        for (int j = 0; j < 3; ++j)
        {
            Vec3f v = ModelBody->vert(face[j]);
            screenCoords[j] = WorldToScreen(ModelBody->vert(face[j]), Width, Height);
            worldCoords[j] = v;
        }

        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * LightDir;
        if (intensity > 0)
        {
            Vec2i uv[3];
            for (int k = 0; k < 3; ++k)
                uv[k] = ModelBody->uv(i, k);
            DrawTriangleZBuffer(screenCoords, uv, ZBuffer, Width, intensity, Image, ModelBody);
        }
    }

    for (int i = 0; i < ModelFace->nfaces(); ++i)
    {
        std::vector<int> face = ModelFace->face(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];

        for (int j = 0; j < 3; ++j)
        {
            Vec3f v = ModelFace->vert(face[j]);
            screenCoords[j] = WorldToScreen(ModelFace->vert(face[j]), Width, Height);
            worldCoords[j] = v;
        }

        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * LightDir;
        if (intensity > 0)
        {
            Vec2i uv[3];
            for (int k = 0; k < 3; ++k)
                uv[k] = ModelFace->uv(i, k);
            DrawTriangleZBuffer(screenCoords, uv, ZBuffer, Width, intensity, Image, ModelFace);
        }
    }

    for (int i = 0; i < ModelHair->nfaces(); ++i)
    {
        std::vector<int> face = ModelHair->face(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];

        for (int j = 0; j < 3; ++j)
        {
            Vec3f v = ModelHair->vert(face[j]);
            screenCoords[j] = WorldToScreen(ModelHair->vert(face[j]), Width, Height);
            worldCoords[j] = v;
        }

        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * LightDir;
        if (intensity > 0)
        {
            Vec2i uv[3];
            for (int k = 0; k < 3; ++k)
                uv[k] = ModelHair->uv(i, k);
            DrawTriangleZBuffer(screenCoords, uv, ZBuffer, Width, intensity, Image, ModelHair);
        }
    }

    for (int i = 0; i < ModelCloak->nfaces(); ++i)
    {
        std::vector<int> face = ModelCloak->face(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];

        for (int j = 0; j < 3; ++j)
        {
            Vec3f v = ModelCloak->vert(face[j]);
            screenCoords[j] = WorldToScreen(ModelCloak->vert(face[j]), Width, Height);
            worldCoords[j] = v;
        }

        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * LightDir;
        if (intensity > 0)
        {
            Vec2i uv[3];
            for (int k = 0; k < 3; ++k)
                uv[k] = ModelCloak->uv(i, k);
            DrawTriangleZBuffer(screenCoords, uv, ZBuffer, Width, intensity, Image, ModelCloak);
        }
    }

    Image.flip_vertically();
    Image.write_tga_file("output.tga");

    delete ModelBody;
    delete ModelFace;
    delete ModelHair;
    delete ModelCloak;
    return 0;
}