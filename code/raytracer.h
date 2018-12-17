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

v3 operator*(v3 & lhs, v3 & rhs)
{
    return { lhs[0]*rhs[0], lhs[1]*rhs[1], lhs[2]*rhs[2] };
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
    Shading_t shadingType;
    float distance;
    v3 point;
    v3 normal;
};


// function prototypes
bool hit(v3 rayOrigin, v3 rayDirection, Hitable* hitables, int hitableCount, HitRecord * hitrec);
v3 color(v3 rayOrigin, v3 rayDirection, Hitable * hitables, int hitableCount, int depth);
float primaryRaySphere(v3 rayOrigin, v3 rayDirection, Sphere object);

#endif RAYTRACER_H