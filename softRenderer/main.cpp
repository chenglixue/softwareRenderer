#include "core.h"
#include <limits>
#include <iostream>

const TGAColor White = TGAColor(255, 255, 255, 255);
const TGAColor Red = TGAColor(255, 0, 0, 255);
const TGAColor RandColor = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);

//the resolution of output of the picture
const int Width = 1920;
const int Height = 1080;
TGAImage Image(Width, Height, TGAImage::RGB);
Vec3f LightDir(0, 0, -1);
std::vector<std::vector<float>> ZBuffer(Height + 1, std::vector<float>(Width + 1,  std::numeric_limits<float>::max()));

//model data
Model* ModelBody = nullptr;
Model* ModelFace = nullptr;

Vec3f world2screen(Vec3f v);

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
    }
    
    //vert : vertex coordinate
    //face : triangle face

    for (int i = 0; i < ModelBody->nfaces(); ++i)
    {
        std::vector<int> face = ModelBody->face(i);

        Vec3f pts[3];

        for (int j = 0; j < 3; ++j)
        {
            pts[j] = world2screen(ModelBody->vert(face[j]));
        }
        DrawTriangleZBuffer(pts, ZBuffer, Width, Image,  TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    for (int i = 0; i < ModelFace->nfaces(); ++i)
    {
        std::vector<int> face = ModelFace->face(i);

        Vec3f pts[3];

        for (int j = 0; j < 3; ++j)
        {
            pts[j] = world2screen(ModelFace->vert(face[j]));
        }
        DrawTriangleZBuffer(pts, ZBuffer, Width, Image, TGAColor(rand()%255, rand()%255, rand()%255, 255));

    }

    Image.flip_vertically();
    Image.write_tga_file("output.tga");

    delete ModelBody;
    delete ModelFace;
    return 0;
}

Vec3f world2screen(Vec3f v) 
{
    return Vec3f(int((v.x + 1.) * Width / 2.), int((v.y + 1.) * Height / 3.), v.z);
}