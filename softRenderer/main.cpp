#include "core.h"

const TGAColor White = TGAColor(255, 255, 255, 255);
const TGAColor Red = TGAColor(255, 0, 0, 255);
const TGAColor RandColor = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);

//the resolution of output of the picture
const unsigned int Width = 436 * 2;
const unsigned int Height = 521 * 2;

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

    DrawWireframeAndSetColor(image, Height, Width, White);

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete ModelBody;
    delete ModelFace;
    return 0;
}