#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string>

#include "raytracer.h"

#define PI 3.1415926f

// TODO(Michael): fix aspect ration when res is not x = y
// TODO(Michael): camera should be struct
// TODO(Michael): viewport should be struct
// TODO(Michael): transform struct for camera and viewport?
// TODO(Michael): figure out why the image is flipped along the y-axis. -> see below where viewSide is computed!
// TODO(Michael): write ppm data into buffer and write to file at the end.
// TODO(Michael): read scene from file

global_var v3 cameraPos = { 0.0f, 0.0f, 40.0f };
global_var v3 lookAt = { 0.0f, 0.0f, 0.0f }; // viewport center
global_var v3 up = { 0, 1, 0 };
global_var float viewPortWidth = 2.0f;
global_var float viewPortHeight = 2.0f;
global_var int resolutionX = 512;
global_var int resolutionY = 512;

inline float randBetween(float lowerBound, float upperBound)
{
    float offset = lowerBound - 0.0f;
    float range = upperBound - lowerBound;
    return range/1.0f * (rand()/(float)RAND_MAX) + lowerBound;
}

void createRandomScene(int hitableCount, Hitable ** hitables, int materialCount, Material ** materials)
{
    *materials = (Material*)malloc(sizeof(Material)*materialCount);
    Material * material = *materials;
    for (int i = 0;
         i < materialCount;
         ++i)
    {
        material->shadingType = METAL;
        material->attenuation = { 
            randBetween(0.3f, 1.0f),
            randBetween(0.3f, 1.0f),
            randBetween(0.3f, 1.0f)}; 
        material->fuzziness = 0.0f; //randBetween(0.0f, 0.4f);
        material++;
    }
    
    *hitables = (Hitable*)malloc(sizeof(Hitable)*hitableCount);
    Hitable * hitable = *hitables;
    for (int i = 0;
         i < hitableCount;
         ++i)
    {
        float materialIndex = randBetween(0, materialCount);
        Material * material = *materials + (int)materialIndex;
        Hitable sphere = {};
        sphere.geometry = SPHERE;
        sphere.material = material;
        sphere.sphere = {
            {
                randBetween(-40.0f, 40.0f),
                randBetween(-20.0f, 20.0f),
                randBetween(-20.0f, 20.0f)
            },
            randBetween(0.1f, 7.0f)
        };
        *hitable = sphere;
        hitable++;
    }
}

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
        hitrec->distance = -1;
    }
    else
    {
        float droot = sqrt(discriminant);
        float root1 = (-b - droot) / 2;
        float root2 = (-b + droot) / 2;
        distance = root1 < root2 ? root1 : root2;
        hitrec->distance = distance;
        hitrec->point = rayOrigin + distance*rayDirection;
        hitrec->normal = normalize(hitrec->point - object.pos);
        if (distance > 0.001f)
            hasHit = true;
    }
    
    return hasHit;
}

bool hit(v3 rayOrigin, v3 rayDirection, Hitable* hitables, int hitableCount, HitRecord * hitrec)
{
    HitRecord tempHitrec;
    float closestSoFar = 0x7FFFFFFF;
    bool hasHit = false;
    Hitable * hitable = hitables;
    for (int i = 0;
         i < hitableCount;
         i++)
    {
        tempHitrec.material = hitable->material;
        switch (hitable->geometry)
        {
            case SPHERE:
            {
                if (hitSphere(rayOrigin, rayDirection, hitable->sphere, &tempHitrec))
                {
                    if (tempHitrec.distance < closestSoFar)
                    {
                        *hitrec = tempHitrec;
                        closestSoFar = hitrec->distance;
                        hasHit = true;
                    }
                }
            }
            break;
        }
        hitable++;
    }
    
    return hasHit;
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

v3 randomInUnitSphere()
{
    v3 one = {1,1,1};
    v3 p;
    v3 test;
    do 
    {
        test = { 
            rand()/float(RAND_MAX),
            rand()/float(RAND_MAX),
            rand()/float(RAND_MAX) };
        p = 2.0f*test - one;
    } while(dot(p, p) >= 1.0f);
    return p;
}

// TODO(Michael): duplicate code
bool scatterLambertianColorizeNormals(HitRecord * hitrec, v3 * attenuation, v3 * scatterDirection)
{
    v3 point = hitrec->point;
    v3 normal = hitrec->normal;
    *attenuation = { normal[0]+1, normal[1]+1 , normal[2]+1 };
    *attenuation = 0.5**attenuation; // NOTE(Michael): hihihihihi
    v3 target = point + normal + randomInUnitSphere();
    *scatterDirection = normalize(target-point);
    return true;
}

bool scatterLambertian(HitRecord * hitrec, v3 * attenuation, v3 * scatterDirection)
{
    *attenuation = hitrec->material->attenuation;
    v3 point = hitrec->point;
    v3 normal = hitrec->normal;
    v3 target = point + normal + randomInUnitSphere();
    *scatterDirection = normalize(target-point);
    return true;
}

v3 reflect(v3 rayDirection, v3 normal)
{
    return (-2*dot(rayDirection, normal)*normal + rayDirection);
}

bool refract(HitRecord * hitrec, v3 rayDirection, v3 * attenuation, v3 * refracted)
{
    float ior = hitrec->material->ior;
    v3 normal = hitrec->normal;
    *attenuation = { 1, 1, 1 };
    v3 incident = normalize(rayDirection);
    float incidentDotNormal = dot(incident, normal);
    float n;
    if (incidentDotNormal < 0) // hit outside surface
    {
        n = 1.0f/ior;
        incidentDotNormal = -incidentDotNormal;
    }
    else // ray inside surface
    {
        n = ior;
        normal = -normal;
    }
    float discriminant = 1-n*n*(1-incidentDotNormal*incidentDotNormal);
    if (discriminant > 0)
    {
        *refracted = normalize(n*(incident + incidentDotNormal*normal) - normal*sqrt(discriminant));
        return true;
    }
    return false;
}

bool scatterMetal(HitRecord * hitrec, v3 rayOrigin, v3 rayDirection, v3 * attenuation, v3 * scatterDirection)
{
    v3 normal = hitrec->normal;
    v3 point = hitrec->point;
    float fuzziness = hitrec->material->fuzziness;
    v3 reflectDirection = reflect(rayDirection, normal);
    *scatterDirection = normalize(reflectDirection + fuzziness*randomInUnitSphere());
    *attenuation = hitrec->material->attenuation;
    return (dot(reflectDirection, normal) > 0);
}

v3 color(v3 rayOrigin, v3 rayDirection, Hitable * hitables, int hitableCount, int depth)
{
    HitRecord hitrec;
    bool hasHit = hit(rayOrigin, rayDirection, hitables, hitableCount, &hitrec);
    if (hasHit)
    {
        if (depth > 0)
        {
            v3 attenuation;
            v3 scatterDirection;
            bool hasScatterHit = false;
            switch (hitrec.material->shadingType)
            {
                case LAMBERT:
                {
                    hasScatterHit = scatterLambertian(&hitrec, &attenuation, &scatterDirection);
                }
                break;
                
                case METAL:
                {
                    hasScatterHit = scatterMetal(&hitrec, rayOrigin, rayDirection, &attenuation, &scatterDirection);
                }
                break;
                
                case DIALECTRIC:
                {
                    hasScatterHit = refract(&hitrec, rayDirection, &attenuation, &scatterDirection);
                }
                break;
                
                case NORMALS:
                {
                    hasScatterHit = scatterLambertianColorizeNormals(&hitrec, &attenuation, &scatterDirection);
                }
                break;
                
                default:
                {
                    return { 1, 0, 0 };
                }
            }
            if (hasScatterHit)
                return attenuation*color(hitrec.point, scatterDirection, hitables, hitableCount, depth-1);
            else
                return { 0, 0, 0 };
        }
        else
        {
            return { 0, 0, 0 };
        }
    }
    else
    {
        v3 one = {1,1,1};
        v3 blue = { 0.3f, 0.5f, 0.8f };
        v3 orange = { 1.0f, float(204)/float(255), float(153)/float(255) };
        float t = 0.5*(rayDirection[1] + 1.0f);
        return ((1.0f - t)*one+t*blue);
    }
}

int main (int argc, char** argv)
{
    // create ppm file, header and buffer
    FILE* ppmFile = fopen("ray_test.ppm", "w");
    std::string resolutionXA = std::to_string(resolutionX);
    std::string resolutionYA = std::to_string(resolutionY);
    std::string ppmHeader = "P3\n" + resolutionXA + " " + resolutionYA + "\n255\n";
    char * outputBuffer = (char *)malloc(sizeof(char)*3*4*(resolutionX*resolutionY) + sizeof(char)*ppmHeader.length());
    if (!outputBuffer)
    {
        return -1;
    }
    char *bufferPos = outputBuffer;
    strcpy(bufferPos, ppmHeader.c_str());
    bufferPos += ppmHeader.length();
    
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
    
    // materials
    Material lambert1 = { LAMBERT, {0.8f, 0.3f, 0.3f} };
    Material lambert2 = { LAMBERT, {float(153)/float(255), 0.0f, float(153)/float(255)} };
    Material metal1 = { METAL, {0.8f, 0.6f, 0.2f} };
    metal1.fuzziness = 0.0f;
    Material metal2 = { METAL, {0.8f, 0.8f, 0.8f} };
    metal2.fuzziness = 0.3f;
    Material metal3 = { METAL, {0.8f, 0.8f, 0.8f} };
    metal3.fuzziness = 0.1f;
    Material glass = { DIALECTRIC, {1,1,1} };
    glass.ior = 1.5f;
    
    // test objects
    Hitable testsphere1 = {};
    testsphere1.geometry = SPHERE;
    testsphere1.material = &lambert1;
    testsphere1.sphere = { {0, 0, -1.0}, 0.5, 1, 0, 1 };
    
    Hitable testsphere2 = {};
    testsphere2.geometry = SPHERE;
    testsphere2.material = &lambert2;
    testsphere2.sphere = { {0, -100.5, -1.0}, 100.0, 1, 0, 1 };
    
    Hitable testsphere3 = {};
    testsphere3.geometry = SPHERE;
    testsphere3.material = &metal1;
    testsphere3.sphere = { {1, 0, -1}, 0.5, 1, 0, 0 };
    
    Hitable testsphere4 = {};
    testsphere4.geometry = SPHERE;
    testsphere4.material = &glass;
    testsphere4.sphere = { {-1, 0, -1}, 0.5f, 1, 0, 1 };
    
    Hitable testsphere5 = {};
    testsphere5.geometry = SPHERE;
    testsphere5.material = &glass;
    testsphere5.sphere = { {-1, 0, -1}, -0.45f, 1, 0, 1 };
    
    // add test objects to "scene"
    Hitable scene[] = { testsphere1, testsphere2, testsphere3, testsphere4 };
    
    // random scene
    Material * randomMaterials = 0;
    int randMaterialCount = 40;
    Hitable * randomHitables = 0;
    int randHitableCount = 100;
    createRandomScene(randHitableCount, &randomHitables, randMaterialCount, &randomMaterials);
    
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
            
            int sampleCount = 1;
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
                v3 originToCenter = cameraPos + viewDir; //*cameraDistance;
                
                // NOTE(Michael): -viewportY, because relative viewport Y goes from top(-) to bottom(+)
                v3 centerToPixel = originToCenter + (viewportX * viewSide) + (-viewportY * viewUp);
                v3 ray = centerToPixel - cameraPos;
                ray = normalize(ray);
                
                int hitableCount = randHitableCount; //sizeof(scene)/sizeof(scene[0]);
                // test intersection with objects
                c = c + color(cameraPos, ray, randomHitables, hitableCount, 5);
            }
            c = c /  float(sampleCount);
            //c = { sqrt(c[0]), sqrt(c[1]), sqrt(c[2]) };
            uint8_t ir = uint8_t(255.99f * c[0]);
            uint8_t ig = uint8_t(255.99f * c[1]);
            uint8_t ib = uint8_t(255.99f * c[2]);
            std::string rgb = std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + " ";
            strcpy(bufferPos, rgb.c_str());
            bufferPos += rgb.length();
        }
        *bufferPos = '\n';
        bufferPos++;
    }
    *bufferPos = '\0';
    bufferPos++;
    int bytesWritten = bufferPos - outputBuffer;
    fwrite(outputBuffer, sizeof(char), bytesWritten, ppmFile);
    fclose(ppmFile);
    
    return 0;
}

