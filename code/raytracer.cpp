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

// check if ray intersects sphere
// see: https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
bool hitSphere(v3 rayOrigin, v3 rayDirection, Sphere object, HitRecord * hitrec)
{
    bool hasHit = false;
    float distance;
    v3 rayOriginToSphereCenter = rayOrigin - object.pos;
    float b = 2 * dot(rayDirection, rayOriginToSphereCenter);
    float c = dot(rayOriginToSphereCenter, rayOriginToSphereCenter) -
        object.radius * object.radius;
    float discriminant = b*b - 4*c;
    if (discriminant < 0) // TODO(Michael): how to init hitrec properly in this case?
    {
        hasHit = false;
        hitrec->distance = -1;
    }
    else
    {
        float droot = sqrt(discriminant);
        float root1 = (-b - droot) / 2;
        float root2 = (-b + droot) / 2;
        distance = root1 < root2 ? root1 : root2;
        hasHit = true;
        hitrec->distance = distance;
        hitrec->point = rayOrigin + distance*rayDirection;
        hitrec->normal = normalize(hitrec->point - object.pos);
    }
    
    return hasHit;
}

HitRecord hit(v3 rayOrigin, v3 rayDirection, Hitable* hitables, int hitableCount)
{
    Hitable * hitable = hitables;
    HitRecord currentHitRec;
    currentHitRec.distance = 0.0f;
    float closestSoFar = 0x7FFFFFFF;
    float d;
    HitRecord tempHitrec;
    for (int i = 0;
         i < hitableCount;
         i++)
    {
        bool hasHit = false;
        tempHitrec.shadingType = hitable->shadingType;
        switch (hitable->geometry)
        {
            case SPHERE:
            {
                hasHit = hitSphere(rayOrigin, rayDirection, hitable->sphere, &tempHitrec);
            }
            break;
        }
        
        if (hasHit)
        {
            if (tempHitrec.distance < closestSoFar && tempHitrec.distance > 0.0f)
            {
                currentHitRec = tempHitrec;
                closestSoFar = currentHitRec.distance;
            }
        }
        hitable++;
    }
    
    //currentHitRec.point = currentHitRec.point + 0.01f * currentHitRec.normal;
    currentHitRec.rayOrigin = rayOrigin;
    currentHitRec.rayDirection = rayDirection;
    return currentHitRec;
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

v3 color(HitRecord * hitrec, Hitable * hitables, int hitableCount)
{
    float distance = hitrec->distance;
    v3 point = hitrec->point;
    v3 normal = hitrec->normal;
    v3 c = { 0.0f, 0.0f, 0.0f };
    switch (hitrec->shadingType)
    {
        case NORMALS:
        {
            c = { normal[0] + 1, normal[1] + 1, normal[2] + 1 };
            c = 0.5f*c;
        }
        break;
        
        case DIFFUSE:
        {
            v3 p;
            v3 one = {1,1,1};
            v3 test;
            do 
            {
                test = { 
                    rand()/float(RAND_MAX),
                    rand()/float(RAND_MAX),
                    rand()/float(RAND_MAX) };
                p = 2.0f*test - one;
            } while(length(p) >= 1.0f);
            v3 target = point + normal + p;
            HitRecord nextHitrec = hit(point, normalize(target - point), hitables, hitableCount);
            if (nextHitrec.distance >= 0.001f)
                return 0.5f*color(&nextHitrec, hitables, hitableCount);
            else
            {
                float t = 0.5*(hitrec->rayDirection[1] + 1.0f);
                v3 blue = { 0.5f, 0.7f, 1.0f };
                return (1.0f - t)*one+t*blue;
                //c = {1,0,0};
            }
        }
        break;
        
        default:
        {
            v3 one = {1,1,1};
            float t = 0.5*(hitrec->rayDirection[1] + 1.0f);
            v3 blue = { 0.5f, 0.7f, 1.0f };
            return (1.0f - t)*one+t*blue;
        }
    }
    return c;
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
    
    // test objects
    Hitable testsphere1 = {};
    testsphere1.geometry = SPHERE;
    testsphere1.shadingType = NORMALS;
    testsphere1.sphere = { {0, 0, -1.0}, 0.5, 1, 0, 1 };
    
    Hitable testsphere2 = {};
    testsphere2.geometry = SPHERE;
    testsphere2.shadingType = DIFFUSE;
    testsphere2.sphere = { {0, -100.5, -1}, 100, 1, 0, 0 };
    
    // add test objects to "scene"
    Hitable scene[2] = { testsphere1, testsphere2 };
    
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
            
            int sampleCount = 500;
            v3 c;
            for (int i = 0;
                 i < sampleCount;
                 i++)
            {
                // compute biases for supersampling
                float colBias = (rand()/float(RAND_MAX));
                float rowBias = (rand()/float(RAND_MAX));
                
                // compute ray from origin to viewport pixel
                float viewportX = pixelWidth * aspectRatio * (float(col)+colBias) - aspectRatio + stepX;
                float viewportY = pixelHeight * (float(row)+rowBias) - 1.0f + stepY;
                
                // NOTE(Michael): if ray is normalized the quadratic equation
                // easier to solve.See
                // https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
                
                // compute ray from camera pos to relative viewport pixel
                v3 originToCenter = cameraPos + viewDir*cameraDistance;
                
                // NOTE(Michael): -viewportY, because relative viewport Y goes from top(-) to bottom(+)
                v3 centerToPixel = originToCenter + (viewportX * viewSide) + (-viewportY * viewUp);
                v3 ray = centerToPixel - cameraPos;
                ray = normalize(ray);
                
                // test intersection with objects
                HitRecord hitrec = hit(cameraPos, ray, scene, 2);
                
                c = c + color(&hitrec, scene, 2);
            }
            c = c /  float(sampleCount);
            uint8_t ir = uint8_t(255.99f * c[0]);
            uint8_t ig = uint8_t(255.99f * c[1]);
            uint8_t ib = uint8_t(255.99f * c[2]);
            std::string rgb = std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + " ";
            fprintf(ppmFile, rgb.c_str());
        }
        fprintf(ppmFile, "\n");
    }
    fclose(ppmFile);
    
    return 0;
}

