#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string>

#include "lang.h"
#include "raytracer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI 3.1415926f

// TODO(Michael): fix aspect ration when res is not x = y
// TODO(Michael): camera should be struct
// TODO(Michael): viewport should be struct
// TODO(Michael): transform struct for camera and viewport?
// TODO(Michael): figure out why the image is flipped along the y-axis. -> see below where viewSide is computed!
// TODO(Michael): write ppm data into buffer and write to file at the end.
// TODO(Michael): read scene from file


global_var float const EPSILON = 0.00001f;

inline float randBetween(float lowerBound, float upperBound)
{
    float range = upperBound - lowerBound;
    return range/1.0f * (rand()/(float)RAND_MAX) + lowerBound;
}

void createRandomScene(uint32_t hitableCount, Hitable ** hitables, uint32_t materialCount, Material ** materials)
{
    *materials = (Material*)malloc(sizeof(Material)*materialCount);
    Material * material = *materials;
    for (uint32_t i = 0;
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
    for (uint32_t i = 0;
         i < hitableCount;
         ++i)
    {
        float materialIndex = randBetween(0, materialCount);
        Material * mat = *materials + (int)materialIndex;
        Hitable sphere;
        sphere.geometry = SPHERE;
        sphere.material = mat;
        sphere.sphere = {
            {
                randBetween(-40.0f, 40.0f),
                randBetween(-20.0f, 20.0f),
                randBetween(-20.0f, 20.0f)
            },
            randBetween(0.1f, 7.0f),
	    0, 0, 0
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
    for (int i = 0; i < hitableCount; i++)
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

	case TRIANGLE:
	{
	    // TODO: Implement
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
    illumination.x = l.r * s.r * dot(normal, pointToLight);
    illumination.y = l.g * s.g * dot(normal, pointToLight);
    illumination.z = l.b * s.b * dot(normal, pointToLight);
    
    return illumination;
}

v3 colorizeNormal(Sphere s, v3 point)
{
    v3 normal = normalize(point - s.pos);
    // mapping components of normalvector from [-1.0,1.0] to [0.0,1.0]
    v3 foo = { normal.x + 1, normal.y + 1, normal.z + 1};
    return 0.5f * foo;
}

v3 randomInUnitSphere()
{
    v3 one = {1,1,1};
    v3 p = {};
    v3 test = {};
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
    *attenuation = { normal.x+1, normal.y+1 , normal.z+1 };
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

bool scatterMetal(HitRecord * hitrec, v3 rayDirection, v3 * attenuation, v3 * scatterDirection)
{
    v3 normal = hitrec->normal;
    float fuzziness = hitrec->material->fuzziness;
    v3 reflectDirection = reflect(rayDirection, normal);
    *scatterDirection = normalize(reflectDirection + fuzziness*randomInUnitSphere());
    *attenuation = hitrec->material->attenuation;
    return (dot(reflectDirection, normal) > 0);
}

bool emit(HitRecord * hitRec, v3 * out_light_color)
{
    *out_light_color = hitRec->material->light_color;
    return true;
}

void init_skybox(Skybox * skybox,
		 char const * negx_file, char const * posx_file,
		 char const * negy_file, char const * posy_file,
		 char const * negz_file, char const * posz_file)
{
    int x, y, n;
    skybox->negx = stbi_load(negx_file, &x, &y, &n, 3);
    skybox->posx = stbi_load(posx_file, &x, &y, &n, 3);
    skybox->negy = stbi_load(negy_file, &x, &y, &n, 3);
    skybox->posy = stbi_load(posy_file, &x, &y, &n, 3);
    skybox->negz = stbi_load(negz_file, &x, &y, &n, 3);
    skybox->posz = stbi_load(posz_file, &x, &y, &n, 3);
    ASSERT(skybox->negx != 0); ASSERT(skybox->negx != 0);
    ASSERT(skybox->negy != 0); ASSERT(skybox->negy != 0);
    ASSERT(skybox->negz != 0); ASSERT(skybox->negz != 0);
    skybox->width = x;
    skybox->height = y;
}

void cleanup_skybox(Skybox * skybox)
{
    ASSERT(skybox->negx != 0); ASSERT(skybox->negx != 0);
    ASSERT(skybox->negy != 0); ASSERT(skybox->negy != 0);
    ASSERT(skybox->negz != 0); ASSERT(skybox->negz != 0);
    stbi_image_free(skybox->negx); stbi_image_free(skybox->posx);
    stbi_image_free(skybox->negy); stbi_image_free(skybox->posy);
    stbi_image_free(skybox->negz); stbi_image_free(skybox->posz);
}

v3 sample_skybox(Skybox skybox, v3 direction)
{
    v3 result;
    uint8_t * skybox_plane = 0;
    direction.y *= -1.f;
    
    if ( abs(direction.x) > abs(direction.y) ) {
	if ( abs(direction.x) > abs(direction.z) ) {
	    if (direction.x > 0) skybox_plane = skybox.posx;
	    else                 skybox_plane = skybox.negx;
	    goto done;
	}
	else {
	    goto choose_z;
	}
    }
    else {
	if ( abs(direction.y) > abs(direction.z) ) {
	    if (direction.y > 0) skybox_plane = skybox.posy;
	    else                 skybox_plane = skybox.negy;
	    goto done;
	}
	else {
	    goto choose_z;
	}
    }

choose_z:   if (direction.z > 0) skybox_plane = skybox.posz;
	    else                 skybox_plane = skybox.negz;    
	
done:
    uint32_t u = (uint32_t)((0.5f*direction.x + 0.5f) * (float)skybox.width);
    uint32_t v = (uint32_t)((0.5f*direction.y + 0.5f) * (float)skybox.height);
    uint8_t r = skybox_plane[3*(v*skybox.width+u)];
    uint8_t g = skybox_plane[3*(v*skybox.width+u)+1];
    uint8_t b = skybox_plane[3*(v*skybox.width+u)+2];
    float fr = 1.f/255.f * (float)r;
    float fg = 1.f/255.f * (float)g;
    float fb = 1.f/255.f * (float)b;
    return v3(fr, fg, fb);
}

v3 color(v3 rayOrigin, v3 rayDirection, Hitable * hitables, int hitableCount, int depth, Skybox skybox)
{
    HitRecord hitrec;
    bool hasHit = hit(rayOrigin, rayDirection, hitables, hitableCount, &hitrec);
    if (hasHit)
    {
        if (depth > 0)
        {
            v3 attenuation;
            v3 scatterDirection;
            v3 light_color = {0, 0, 0};
            bool emitted = false;
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
                    hasScatterHit = (&hitrec, rayDirection, &attenuation, &scatterDirection);
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
                
                case DIFFUSE_LIGHT:
                {
                    emitted = emit(&hitrec, &light_color);
                }
                break;               
            }
            if (hasScatterHit)
                return attenuation*color(hitrec.point, scatterDirection, hitables, hitableCount, depth-1, skybox);
            else
                return light_color;
        }
        else // depth == 0
        {
            return { 0, 0, 0 };
        }
    } // if (hasHit)
    else
    {
	return sample_skybox(skybox, rayDirection);
        // v3 one = {1,1,1};
        // v3 gray = {.2f, .2f, .2f};
        // v3 blue = { 0.3f, 0.5f, 0.8f };
        // v3 orange = { 1.0f, float(204)/float(255), float(153)/float(255) };
        // float t = 0.5f*(rayDirection.y + 1.0f);
        // return ((1.0f - t)*one+t*gray);
        
        //return {0,0,0};
    }
}

int main (int argc, char** argv)
{
    global_var v3 cameraPos( 0.5f, 0.f, -.5f );
    global_var v3 lookAt( 0.0f, 0.0f, 0.0f ); // viewport center
    global_var v3 up( 0, 1, 0 );
    global_var float viewPortWidth = 2.0f;
    global_var float viewPortHeight = 2.0f;
    global_var uint32_t resolutionX = 512;
    global_var uint32_t resolutionY = 512;

    // create ppm file, header and buffer
    FILE* ppmFile;
    fopen_s(&ppmFile, "ray_test.ppm", "w");
    std::string resolutionXA = std::to_string(resolutionX);
    std::string resolutionYA = std::to_string(resolutionY);
    std::string ppmHeader = "P3\n" + resolutionXA + " " + resolutionYA + "\n255\n";
    uint32_t outputSize = sizeof(char)*3*4*(resolutionX*resolutionY) + (uint32_t)(sizeof(char)*ppmHeader.length());
    char * outputBuffer = (char *)malloc(outputSize);
    if (!outputBuffer)
    {
        return -1;
    }
    char *bufferPos = outputBuffer;
    strcpy_s(bufferPos, outputSize, ppmHeader.c_str());
    bufferPos += ppmHeader.length();
    
    // compute view, right and up vectors via Gram-Schmidt orthogonalization (p. 236 Van Verth)
    v3 viewDir = lookAt - cameraPos;
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
    Material lambert1 = { LAMBERT, v3(0.8f, 0.3f, 0.3f), {} };
    Material lambert2 = { LAMBERT, v3(0.1f, 0.0f, .7f), {} };
    Material metal1 = { METAL, v3(1.0f, 1.0f, 1.0f), {} };
    metal1.fuzziness = 0.0f;
    Material metal2 = { METAL, v3(1.f, 0.84f, .0f), {} };
    metal2.fuzziness = 0.3f;
    Material glass = { DIALECTRIC, v3(1,1,1), {} };
    glass.ior = 1.5f;
    Material light = {DIFFUSE_LIGHT, v3(), {}};
    Material light2 = {DIFFUSE_LIGHT, v3(), {}};
    light.light_color = v3(4, 4, 4);
    light2.light_color = v3(4, 4, 4);
    
    // test objects
    Hitable testsphere1 = {};
    testsphere1.geometry = SPHERE;
    testsphere1.material = &lambert1;
    testsphere1.sphere = { {0, 0, 2}, 0.5, 1, 0, 0 };
    
    Hitable testsphere2 = {};
    testsphere2.geometry = SPHERE;
    testsphere2.material = &lambert2;
    testsphere2.sphere = { {0, -100.5, 2}, 100.0, 1, 0, 0 };
    
    Hitable testsphere3 = {};
    testsphere3.geometry = SPHERE;
    testsphere3.material = &metal1;
    testsphere3.sphere = { {0, 0, 0}, 0.25, 1, 0, 0 };
    
    Hitable testsphere4 = {};
    testsphere4.geometry = SPHERE;
    testsphere4.material = &metal2;
    testsphere4.sphere = { {-1, 0, 2}, 0.5f, 0, 0, 0};
    
    Hitable lightsphere1 = {};
    lightsphere1.geometry = SPHERE;
    lightsphere1.material = &light;
    lightsphere1.sphere = { {0, 0, 0}, 1.0f, 0, 0, 0 };
    
    Hitable lightsphere2 = {};
    lightsphere2.geometry = SPHERE;
    lightsphere2.material = &light2;
    lightsphere2.sphere = { {-1.f, 2.7f, 3.f }, .2f, 0, 0, 0 };
    
    Hitable lightsphere3 = {};
    lightsphere3.geometry = SPHERE;
    lightsphere3.material = &light;
    lightsphere3.sphere = { {-2, 3, 5 }, 1.0f, 0, 0, 0 };
    
    Hitable lightsphere4 = {};
    lightsphere4.geometry = SPHERE;
    lightsphere4.material = &light;
    lightsphere4.sphere = { {-2, 3, 3 }, .2f, 0, 0, 0 };
    
    
    // add test objects to "scene"
    Hitable scene[] = { testsphere3 };
    
    // random scene
    Material * randomMaterials = nullptr;
    uint32_t randMaterialCount = 40;
    Hitable * randomHitables = nullptr;
    uint32_t randHitableCount = 100;
    createRandomScene(randHitableCount, &randomHitables, randMaterialCount, &randomMaterials);
    
    // test lights
    Light testLight;
    testLight.pos = { -10.0f, 5.0f, 3.0f };
    testLight.r = 1.0f;
    testLight.g = 1.0f;
    testLight.b = 1.0f;

    //skybox
    Skybox skybox;
    init_skybox(&skybox,
		"../textures/beach_signed/negx.jpg",
		"../textures/beach_signed/posx.jpg",
		"../textures/beach_signed/negy.jpg",
		"../textures/beach_signed/posy.jpg",
		"../textures/beach_signed/negz.jpg",
		"../textures/beach_signed/posz.jpg");
    printf("Skybox width/height: (%d, %d)\n", skybox.width, skybox.height);
    
    for (uint32_t row = 0; row < resolutionY; ++row)
    {
        float percent_render_done = ((float)row/resolutionY)*100.f;
        if (!(row%5)) {
            printf("rendered: %.2f\n", (float)percent_render_done);
        }
        for (uint32_t col = 0; col < resolutionX; ++col)
        {
            
            uint32_t sampleCount = 100;
            v3 c;
            for (uint32_t i = 0; i < sampleCount; i++)
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
                
                int hitableCount = sizeof(scene)/sizeof(scene[0]);
                // test intersection with objects
                c = c + color(cameraPos, ray, scene, hitableCount, 10, skybox);
            }
            c = c /  float(sampleCount);
            c = clamp_v3(c, 1.f);
            //c = { sqrt(c[0]), sqrt(c[1]), sqrt(c[2]) };
            uint8_t ir = uint8_t(255.99f * c.x);
            uint8_t ig = uint8_t(255.99f * c.y);
            uint8_t ib = uint8_t(255.99f * c.z);
            std::string rgb = std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + " ";
            strcpy_s(bufferPos, outputSize, rgb.c_str());
            bufferPos += rgb.length();
        }
        *bufferPos = '\n';
        bufferPos++;
    }
    *bufferPos = '\0';
    bufferPos++;
    size_t bytesWritten = (size_t)bufferPos - (size_t)outputBuffer;
    fwrite(outputBuffer, sizeof(char), bytesWritten, ppmFile);
    fclose(ppmFile);
    cleanup_skybox(&skybox);
    
    return 0;
}

