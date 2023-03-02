#include "Core.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

//the resolution of output of the picture
const uint32_t width = 2560;
const uint32_t height = width * 9 / 16;

int main()
{
    TGAImage image(width, height, TGAImage::RGB);
    line(0, 0, width, height, image, white);
    image.write_tga_file("output.tga");
    return 0;
}