#ifndef RAYTRACER_H
#define RAYTRACER_H


#define global_var static

struct v3
{
    float components[3] = { };
    
    
    float& operator[](int index)
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

enum Shading_t
{
    DIFFUSE,
    NORMALS
};

enum Geometry_t
{
    SPHERE
};

struct Light
{
    v3 pos;
    float r, g, b;
};

struct Sphere
{
    v3 pos;
    float radius;
    float r, g, b; // is this the diffuse?
};


struct Hitable
{
    Geometry_t geometry;
    Shading_t shadingType;
    union
    {
        Sphere sphere;
    };
};

struct HitRecord
{
    float distance;
    v3 point;
    v3 normal;
    Shading_t shadingType;
};


// function prototypes
v3 color(HitRecord * hitrec);
float primaryRaySphere(v3 rayOrigin, v3 rayDirection, Sphere object);

#endif RAYTRACER_H