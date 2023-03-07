#include "core.h"

const TGAColor White = TGAColor(255, 255, 255, 255);
const TGAColor Red = TGAColor(255, 0, 0, 255);
const TGAColor RandColor = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);

//the resolution of output of the picture
const unsigned int Width = 436 * 2;
const unsigned int Height = 521 * 2;
Vec3f LightDir(0.0, 0.0, -1.0);
int* ZBuffer = new int[Width * Height];

//model data
Model* ModelBody = nullptr;
Model* ModelFace = nullptr;

TGAImage image(Width, Height, TGAImage::RGB);

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

    //init zBuffer
    for (int i = Width * Height; i >= 0; i--)
        zBuffer[i] = -std::numeric_limits<float>::max();

    output(image, Height, Width, White);

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete ModelBody;
    delete ModelFace;
    return 0;
}