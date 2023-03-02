#include "core.h"
/*
bool isInsideTriangle( Vec3f* v, float x, float y )
{
    Vec2f side1 = { v[1].x - v[0].x, v[1].y - v[0].y };
    Vec2f side2 = { v[2].x - v[1].x, v[2].y - v[1].y };
    Vec2f side3 = { v[0].x - v[2].x, v[0].y - v[2].y };

    Vec2f v1 = { x - v[0].x, y - v[0].y };
    Vec2f v2 = { x - v[1].x, y - v[1].y };
    Vec2f v3 = { x - v[2].x, y - v[3].y };

    float z1 = side1.x * v1.y - v1.x * side1.y;
    float z2 = side2.x * v2.y - v2.x * side2.y;
    float z3 = side3.x * v3.y - v3.y * side3.y;

    if( ( z1 > 0 && z2 > 0 && z3 > 0) || ( z1 < 0 && z2 < 0 && z3 < 0 ) ) return true;
    return false;
}

void drawTriangle( Vec3f* v, TGAImage& image, TGAColor color )
{

    int minX = std::min( v[0].x, std::min( v[1].x, v[2].x ) );
    int maxX = std::max( v[0].x, std::max( v[1].x, v[2].x ) );
    int minY = std::min( v[0].y, std::min( v[1].y, v[2].y ) );
    int maxY = std::max( v[0].y, std::max( v[1].y, v[2].y ) );

    for( int i = minX; i <= maxX; ++i )
    {
        for( int j = minY; j <= minY; ++j )
        {
            if( isInsideTriangle( v, i, j ) == true ) image.set( i, j, color );
        }
    }


}

std::tuple<float, float, float> computeBarycentric2D(float x, float y, Vec3f* v){
    float c1 = (x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * y + v[1].x * v[2].y - v[2].x*v[1].y) / (v[0].x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * v[0].y + v[1].x * v[2].y - v[2].x * v[1].y );
    float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x*v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y );
    float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x*v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y );
    return {c1,c2,c3};
}
*/

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
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