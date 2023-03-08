#include "core.h"

void Line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
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

void DrawTriangle(Vec2i* v, TGAImage& image, const TGAColor& color)
{
    if (v[0].y > v[1].y) std::swap(v[0], v[1]);
    if (v[0].y > v[2].y) std::swap(v[0], v[2]);
    if (v[1].y > v[2].y) std::swap(v[1], v[2]);

    int totalHeight = v[2].y - v[0].y + 1;

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

void DrawTriangleZBuffer(Vec3f* v, Vec2i* uv,std::vector<std::vector<int>>& zBuffer, int width, float intens, TGAImage& image, Model* model)
{
    const int minX = std::floor(std::min( v[0].x, std::min( v[1].x, v[2].x ) ));
    const int maxX = std::ceil(std::max( v[0].x, std::max( v[1].x, v[2].x ) ));
    const int minY = std::floor(std::min( v[0].y, std::min( v[1].y, v[2].y ) ));
    const int maxY = std::ceil(std::max( v[0].y, std::max( v[1].y, v[2].y ) ));

    for( int i = minX; i <= maxX; ++i )
    {
        for( int j = minY; j <= maxY; ++j )
        {
            if( IsInsideTriangle( v, i + 0.5f, j + 0.5f ) == true ) 
            {
                //重心坐标插值求z值
                auto bc_screen = ComputeBarycentric( v, i + 0.5f, j + 0.5f );
                float tempZ = v[0].z * bc_screen.x / 1.0f + v[1].z * bc_screen.y / 1.0f + v[2].z * bc_screen.z / 1.0f;
                Vec2i uvP = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;

                //更新z值
                if( tempZ > zBuffer[j][i] )
                {
                    zBuffer[j][i] = tempZ;
                    TGAColor color = model->diffuse(uvP);
                    image.set( i, j, TGAColor(color.r * intens, color.g * intens, color.b * intens ) );
                }

            }

        }
    }
}