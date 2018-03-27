#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>

// TODO(Michael): camera should be struct
// TODO(Michael): viewport should be struct
// TODO(Michael): transform struct for camera and viewport?
// TODO(Michael): figure out why the image is flipped along the y-axis. -> see below where viewSide is computed!
// TODO(Michael): write ppm data into buffer and write to file at the end.
// TODO(Michael): read scene from file

#define global_var static

struct v3
{
    float components[3] = { };
    
    float operator[](int index)
    {
        return components[index];
    }
    
    v3 operator-(v3 b)
    {
        return {
            components[0] - b[0],
            components[1] - b[1],
            components[2] - b[2]
        };
    }
    
    v3 operator+(v3 b)
    {
        return {
            components[0] + b[0],
            components[1] + b[1],
            components[2] + b[2]
        };
    }
    
    v3 operator/(float scalar)
    {
        return {
            components[0] / scalar,
            components[1] / scalar,
            components[2] / scalar
        };
    }
    
    v3 operator*(float scalar)
    {
        return {
            components[0] * scalar,
            components[1] * scalar,
            components[2] * scalar
        };
    }
};

v3 operator*(float scalar, v3 v)
{
    return {
        v[0] * scalar,
        v[1] * scalar,
        v[2] * scalar
    };
}

struct sphere
{
    v3 pos;
    float radius;
};

global_var v3 cameraPos = { 3.0f, 0.0f, 5.0f };
global_var v3 lookAt = { 0.0f, 0.0f, 0.0f }; // viewport center
global_var v3 up = { 0, 1, 0 };
global_var float viewPortWidth = 2.0f;
global_var float viewPortHeight = 2.0f;
global_var int resolutionX = 32;
global_var int resolutionY = 32;

v3 cross(v3 a, v3 b)
{
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

float dot(v3 a, v3 b)
{
    return
        a[0] * b[0] +
        a[1] * b[1] +
        a[2] * b[2];
}

float length(v3 v)
{
    return sqrt(
        v[0] * v[0] +
        v[1] * v[1] +
        v[2] * v[2]);
}

v3 normalize(v3 a)
{
    return a / length(a);
}

float primaryRay(v3 rayOrigin, v3 rayDirection, sphere object)
{
    float b = 2 * (rayDirection[0] * (rayOrigin[0] - object.pos[0]) +
                   rayDirection[1] * (rayOrigin[1] - object.pos[1]) +
                   rayDirection[2] * (rayOrigin[2] - object.pos[2])
                   );
    float c = 
        (rayOrigin[0] - object.pos[0]) * (rayOrigin[0] - object.pos[0]) +
        (rayOrigin[1] - object.pos[1]) * (rayOrigin[1] - object.pos[1]) +
        (rayOrigin[2] - object.pos[2]) * (rayOrigin[2] - object.pos[2]) -
        object.radius * object.radius;
    
    float discriminant = b*b - 4*c;
    if (discriminant < 0)
        return -1;
    
    float droot = sqrt(discriminant);
    float root1 = (-b - droot) / 2;
    if (root1 > 0)
        return root1;
    return (-b + droot) / 2;
}

int main (int argc, char** argv)
{
    // create ppm file and header
    FILE* ppmFile = fopen("ray_test.ppm", "w");
    std::string resolutionXA = std::to_string(resolutionX);
    std::string resolutionYA = std::to_string(resolutionY);
    std::string ppmHeader = "P3\n" + resolutionXA + " " + resolutionYA + "\n255\n";
    fprintf(ppmFile, ppmHeader.c_str());
    
    // compute view, right and up vectors via Gram-Schmidt orthogonalization (p. 236 Van Verth)
    v3 viewDir = lookAt - cameraPos;
    float cameraDistance = length(viewDir);
    viewDir = normalize(viewDir);
    v3 viewUp = normalize(up - (viewDir * dot(up, viewDir)));
    v3 viewSide = cross(viewDir, viewUp); // TODO(Michael): righ or left handed system? FIGURE THIS OUT!!!!
    
    // viewport attributes
    float pixelWidth  = viewPortWidth / resolutionX;
    float pixelHeight = viewPortHeight / resolutionY;
    float stepX = pixelWidth  / 2.0f;
    float stepY = pixelHeight / 2.0f;
    
    // test object
    sphere testSphere;
    testSphere.pos = { 0.0f, 0.0f, 0.0f };
    testSphere.radius = 1.0f;
    
    for (int row = 0;
         row < resolutionY;
         ++row)
    {
        for (int col = 0;
             col < resolutionX;
             ++col)
        {
            // compute ray from origin to viewport pixel
            float viewportX = pixelWidth * col - 1.0f + stepX;
            float viewportY = pixelHeight * row - 1.0f + stepY;
            float viewportZ = 0; // // NOTE(Michael): depends on camera pos
            v3 viewPortVec = { viewportX, viewportY, viewportZ };
            //viewPortVec = cameraPos + viewPortVec;
            
            // NOTE(Michael): if ray is normalized the quadratic equation
            // easier to solve.See
            // https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
            
            // compute ray from camera pos to relative viewport pixel
            v3 originToCenter = cameraPos +  cameraDistance * viewDir;
            // NOTE(Michael): -viewportY, because relative viewport Y goes from top(-) to bottom(+)
            v3 centerToPixel = originToCenter + (viewportX * viewSide) + (-viewportY * viewUp);
            v3 ray = centerToPixel - cameraPos;
            ray = normalize(ray);
            
            // test intersection with objects
            float distance = primaryRay(cameraPos, ray, testSphere);
            
#define toTerminal 1
            // print to console
            if (distance > 0)
            {
#if toTerminal
                printf ("X ");
#endif
                fprintf(ppmFile, "255 255 255 ");
            }
            else
            {
#if toTerminal
                printf ("- ");
#endif
                fprintf(ppmFile, "0   0   0   ");
            }
        }
#if toTerminal
        printf ("\n");
#endif
        fprintf(ppmFile, "\n");
    }
    fclose(ppmFile);
    
    return 0;
}

