#include "core.h"
#include <vector>
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

//the resolution of output of the picture
const uint32_t width = 436 * 2;
const uint32_t height = 521 * 2;

//model data
Model* modelBody = nullptr;
Model* modelFace = nullptr;

void outputModel(Model* model, int argc, char** argv);

int main(int argc, char** argv)
{
    outputModel(modelBody, argc, argv);
    return 0;
}

// v : vertex
//f : Three vertex coordinates
void outputModel(Model* model, int argc, char** argv)
{
    if (2 == argc) 
    {
        modelBody = new Model(argv[1]);
        modelFace = new Model(argv[1]);
    }
    else 
    {
        modelBody = new Model("obj/fuhuabody.obj");
        modelFace = new Model("obj/fuhuaface.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);

    //vert : vertex coordinate
    //face : triangle face
    for (int i = 0; i < modelBody->nfaces(); ++i)
    {
        std::vector<int> face = modelBody->face(i);

        for (int j = 0; j < 3; ++j)
        {
            auto v0 = modelBody->vert(face[j]);
            auto v1 = modelBody->vert(face[(j + 1) % 3]);

            //World space to screen space
            int x0 = (v0.x + 1.) * width / 2.;
            int x1 = (v1.x + 1.) * width / 2.;
            int y0 = (v0.y + 0.5) * height / 3.5;
            int y1 = (v1.y + 0.5) * height / 3.5;

            line(x0, y0, x1, y1, image, white);
        }
    }

    for (int i = 0; i < modelFace->nfaces(); ++i)
    {
        std::vector<int> face = modelFace->face(i);

        for (int j = 0; j < 3; ++j)
        {
            auto v0 = modelFace->vert(face[j]);
            auto v1 = modelFace->vert(face[(j + 1) % 3]);

            //World space to screen space
            int x0 = (v0.x + 1.) * width / 2.;
            int x1 = (v1.x + 1.) * width / 2.;
            int y0 = (v0.y + 0.5) * height / 3.5;
            int y1 = (v1.y + 0.5) * height / 3.5;

            line(x0, y0, x1, y1, image, white);
        }
    }

    
    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete modelBody;
    delete modelFace;
}