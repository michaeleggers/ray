#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string>

#include "raytracer.h"

// TODO(Michael): fix aspect ration when res is not x = y
// TODO(Michael): camera should be struct
// TODO(Michael): viewport should be struct
// TODO(Michael): transform struct for camera and viewport?
// TODO(Michael): figure out why the image is flipped along the y-axis. -> see below where viewSide is computed!
// TODO(Michael): write ppm data into buffer and write to file at the end.
// TODO(Michael): read scene from file

global_var v3 cameraPos = { 0.0f, 0.0f, 2.0f };
global_var v3 lookAt = { 0.0f, 0.0f, 0.0f }; // viewport center
global_var v3 up = { 0, 1, 0 };
global_var float viewPortWidth = 2.0f;
global_var float viewPortHeight = 2.0f;
global_var int resolutionX = 1920;
global_var int resolutionY = 817;

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

// check if ray intersects sphere
// see: https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
float primaryRaySphere(v3 rayOrigin, v3 rayDirection, Sphere object)
{
    v3 rayOriginToSphereCenter = rayOrigin - object.pos;
    float b = 2 * dot(rayDirection, rayOriginToSphereCenter);
    float c = dot(rayOriginToSphereCenter, rayOriginToSphereCenter) -
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

v3 diffuse(Light l, Sphere s, v3 point)
{
    v3 normal = normalize(point - s.pos);
    v3 pointToLight = normalize(l.pos - point); // TODO(Michael): why -1?
    v3 illumination;
    illumination[0] = l.r * s.r * dot(normal, pointToLight);
    illumination[1] = l.g * s.g * dot(normal, pointToLight);
    illumination[2] = l.b * s.b * dot(normal, pointToLight);
    
    return illumination;
}

v3 colorizeNormal(Sphere s, v3 point)
{
    v3 normal = normalize(point - s.pos);
    // mapping components of normalvector from [-1.0,1.0] to [0.0,1.0]
    v3 foo = { normal[0] + 1, normal[1] + 1, normal[2] + 1};
    return 0.5f * foo;
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
    float cameraDistance = length(viewDir); // TODO(Michael): LOL!
    viewDir = normalize(viewDir);
    v3 viewUp = normalize(up - (viewDir * dot(up, viewDir)));
    v3 viewSide = cross(viewDir, viewUp); // TODO(Michael): righ or left handed system? FIGURE THIS OUT!!!!
    // NOTE(Michael): https://en.wikipedia.org/wiki/Right-hand_rule
    
    // viewport attributes
    float aspectRatio = (float)resolutionX / (float)resolutionY;
    float pixelWidth  = viewPortWidth / resolutionX;
    float pixelHeight = viewPortHeight / resolutionY;
    float stepX = pixelWidth / 2.0f;
    float stepY = pixelHeight / 2.0f;
    
    // test object
    Hitable testsphereHtbl = {};
    Sphere testsphere;
    testsphere.pos = { 0.0f, 0.0f, 0.0f };
    testsphere.radius = 1.0f;
    testsphere.r = 1.0f;
    testsphere.g = 0.0f;
    testsphere.b = 1.0f;
    testsphere.shadingType = NORMALS;
    testsphereHtbl.geometryData.sph = testsphere;
    testsphereHtbl.geometry = SPHERE;
    
    // test lights
    Light testLight;
    testLight.pos = { -10.0f, 5.0f, 3.0f };
    testLight.r = 1.0f;
    testLight.g = 1.0f;
    testLight.b = 1.0f;
    
    for (int row = 0;
         row < resolutionY;
         ++row)
    {
        for (int col = 0;
             col < resolutionX;
             ++col)
        {
            // compute ray from origin to viewport pixel
            float viewportX = pixelWidth * aspectRatio * col - aspectRatio + stepX;
            float viewportY = pixelHeight * row - 1.0f + stepY;
            float viewportZ = 0; // // NOTE(Michael): depends on camera pos
            v3 viewPortVec = { viewportX, viewportY, viewportZ };
            //viewPortVec = cameraPos + viewPortVec;
            
            // NOTE(Michael): if ray is normalized the quadratic equation
            // easier to solve.See
            // https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
            
            // compute ray from camera pos to relative viewport pixel
            v3 originToCenter = cameraPos + viewDir;
            // NOTE(Michael): -viewportY, because relative viewport Y goes from top(-) to bottom(+)
            v3 centerToPixel = originToCenter + (viewportX * viewSide) + (-viewportY * viewUp);
            v3 ray = centerToPixel - cameraPos;
            ray = normalize(ray);
            
            // test intersection with objects
            float distance = primaryRaySphere(cameraPos, ray, testsphereHtbl.geometryData.sph);
            // float distance = primaryRaySphere(cameraPos, ray, testSphere);
            
#define toTerminal 0
            // print to console
            if (distance > 0)
            {
#if toTerminal
                printf ("X ");
#endif
                
                float r, g, b;
                switch (testsphereHtbl.geometryData.sph.shadingType)
                {
                    case DIFFUSE:
                    {
                        v3 diffuseReflection = diffuse(testLight, testsphereHtbl.geometryData.sph, cameraPos + (distance * ray));
                        r = testsphereHtbl.geometryData.sph.r * 0.4 + 0.6 * diffuseReflection[0];
                        g = testsphereHtbl.geometryData.sph.g * 0.4 + 0.6 * diffuseReflection[1];
                        b = testsphereHtbl.geometryData.sph.b * 0.4 + 0.6 * diffuseReflection[2];
                        if (r > 1.0f) r = 1.0f;
                        if (g > 1.0f) g = 1.0f;
                        if (b > 1.0f) b = 1.0f;
                        if (r < 0.0f) r = 0.0f;
                        if (g < 0.0f) g = 0.0f;
                        if (b < 0.0f) b = 0.0f;
                    }
                    break;
                    
                    case NORMALS:
                    {
                        v3 normalColor = colorizeNormal(testsphereHtbl.geometryData.sph, cameraPos + (distance * ray));
                        r = normalColor[0];
                        g = normalColor[1];
                        b = normalColor[2];
                    }
                    break;
                    
                    default:
                    {
                        r = 0.0f;
                        g = 0.0f;
                        b = 0.0f;
                    }
                }
                
                uint8_t ir = 255.99f * r;
                uint8_t ig = 255.99f * g;
                uint8_t ib = 255.99f * b;
                
                std::string rgb = std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + " ";
                fprintf(ppmFile, rgb.c_str());
            }
            else
            {
                v3 white = { 1.0f, 1.0f, 1.0f };
                v3 lightBlue = { 0.5f, 0.7f, 1.0f };
                float t = 0.5f*(ray[1] + 1.0f);
                v3 color = (1.0 - t)*white + t*lightBlue;
                uint8_t ir = 255.99f * color[0];
                uint8_t ig = 255.99f * color[1];
                uint8_t ib = 255.99f * color[2];
                std::string background = std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + " ";
#if toTerminal
                printf ("- ");
#endif
                fprintf(ppmFile, background.c_str());
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

