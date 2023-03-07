#include "core.h"

void Line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) 
{
    bool steep = false; //Whether swap coordinates x and Y
    //When the slope is bigger than 1, swap coordinates x , y.Otherwise there will be holes in the line segment
    if (std::abs(y0 - y1) > std::abs(x0 - x1)) 
    {

        std::swap(x0, y0);
        std::swap(x1, y1);

        steep = true;
    }

    //When the X-coordinate of the starting point is bigger than the X-coordinate of the ending point,swap them.Otherwise, can't draw a line
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    //Algorithm optimization.use int instead of float 
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror = std::abs(2 * dy);
    int error = 0;

    for (int x = x0, y = y0; x <= x1; ++x)
    {
        //Whether  swap coordinates x and Y
        if (steep == true)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }

        //Bresenham's line
        error += derror;
        if (error > 0)
        {

            y += y0 > y1 ? -1 : 1;
            error -= 2 * dx;

        }
    }
}

void fillTriangle(Vec2i* v, TGAImage& image, const TGAColor& color)
{
    if (v[0].y > v[1].y) std::swap(v[0], v[1]);
    if (v[0].y > v[2].y) std::swap(v[0], v[2]);
    if (v[1].y > v[2].y) std::swap(v[1], v[2]);

    int totalHeight = v[2].y - v[0].y;

    for (int _y = v[0].y; _y <= v[1].y; ++_y)
    {
        int target = v[1].y - v[0].y + 1;
        float alpha = (float)(_y - v[0].y) / totalHeight;
        float beta = (float)(_y - v[0].y) / target;

        Vec2i A = v[0] + (v[2] - v[0]) * alpha;
        Vec2i B = v[0] + (v[1] - v[0]) * beta;

        if (A.x > B.x) std::swap(A, B);

        for (int i = A.x; i <= B.x; ++i)
            image.set(i, _y, color);
    }

    for (int _y = v[1].y; _y <= v[2].y; ++_y)
    {
        int target = v[2].y - v[1].y + 1;
        float alpha = (float)(_y - v[0].y) / totalHeight;
        float beta = (float)(_y - v[1].y) / target;

        Vec2i A = v[0] + (v[2] - v[0]) * alpha;
        Vec2i B = v[1] + (v[2] - v[1]) * beta;

        if (A.x > B.x) std::swap(A, B);

        for (int i = A.x; i <= B.x; ++i)
            image.set(i, _y, color);
    }
}

void output(TGAImage& image, const unsigned int& height, const unsigned int& width, const TGAColor& color)
{
    //vert : vertex coordinate
    //face : triangle face
    for (int i = 0; i < ModelBody->nfaces(); ++i)
    {
        std::vector<int> face = ModelBody->face(i);
        Vec3f worldCoords[3];

        Vec2i screenCoords[3];

        for (int j = 0; j < 3; ++j)
        {
            auto v = ModelBody->vert(face[j]);
            worldCoords[j] = v;

            screenCoords[j] = Vec2i((v.x + 1.) * width / 2., (v.y * 0.5 + 1.) * height / 2.);
        }

        // draw wireframe
        Line(screenCoords[0].x, screenCoords[0].y, screenCoords[1].x, screenCoords[1].y, image, color);
        Line(screenCoords[1].x, screenCoords[1].y, screenCoords[2].x, screenCoords[2].y, image, color);
        Line(screenCoords[2].x, screenCoords[2].y, screenCoords[0].x, screenCoords[0].y, image, color);

        //light
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * LightDir;
        if(intensity > 0)
            fillTriangle(screenCoords, image, TGAColor(intensity * 255, intensity *  255, intensity *  255, 255));
        
    }

    for (int i = 0; i < ModelFace->nfaces(); ++i)
    {
        std::vector<int> face = ModelFace->face(i);

        Vec3f worldCoords[3];
        Vec2i screenCoords[3];

        for (int j = 0; j < 3; ++j)
        {
            auto v = ModelFace->vert(face[j]);
            worldCoords[j] = v;

            screenCoords[j] = Vec2i((v.x + 1.) * width / 2., (v.y * 0.5 + 1.) * height / 2.);
        }

        Line(screenCoords[0].x, screenCoords[0].y, screenCoords[1].x, screenCoords[1].y, image, color);
        Line(screenCoords[1].x, screenCoords[1].y, screenCoords[2].x, screenCoords[2].y, image, color);
        Line(screenCoords[2].x, screenCoords[2].y, screenCoords[0].x, screenCoords[0].y, image, color);
        
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * LightDir;
        if (intensity > 0)
            fillTriangle(screenCoords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
    }

}