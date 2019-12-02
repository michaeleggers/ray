#ifndef RAYTRACER_H
#define RAYTRACER_H

#define max(a, b) (a > b ? a : b)

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
    
    v3 operator*(v3 & rhs)
    {
        return { components[0]*rhs[0], components[1]*rhs[1], components[2]*rhs[2] };
    }
    
};

v3 operator-(v3 & v)
{
    return { 
        -v.components[0],
        -v.components[1],
        -v.components[2]
    };
}

v3 operator*(float scalar, v3 v)
{
    return {
        v[0] * scalar,
        v[1] * scalar,
        v[2] * scalar
    };
}

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

v3 clamp_v3(v3 a, float c)
{
    return {
        a[0] > c ? c : a[0],
        a[1] > c ? c : a[1],
        a[2] > c ? c : a[2],
    };
}

struct Triangle
{
    v3 v0, v1, v2;
};

enum Shading_t
{
    LAMBERT,
    METAL,
    DIALECTRIC,
    DIFFUSE_LIGHT,
    NORMALS
};

enum Geometry_t
{
    SPHERE,
    TRIANGLE
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

struct Material
{
    Shading_t shadingType;
    v3 attenuation;
    union
    {
        float fuzziness;
        float ior;
        v3 light_color;
    };
};

struct Hitable
{
    Geometry_t geometry;
    Material * material;
    union
    {
        Sphere sphere;
        Triangle triangle;
    };
};

struct HitRecord
{
    Material * material;
    float distance;
    v3 point;
    v3 normal;
};


// function prototypes
bool hit(v3 rayOrigin, v3 rayDirection, Hitable* hitables, int hitableCount, HitRecord * hitrec);
v3 color(v3 rayOrigin, v3 rayDirection, Hitable * hitables, int hitableCount, int depth);
float primaryRaySphere(v3 rayOrigin, v3 rayDirection, Sphere object);

#endif RAYTRACER_H