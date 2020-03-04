#ifndef RAYTRACER_H
#define RAYTRACER_H

#define max(a, b) (a > b ? a : b)

#define global_var static

struct v3
{
    v3() { x = y = z = 0; }
    
    v3(float _x, float _y, float _z) {
	x = _x; y = _y; z = _z;
    }
    
    float x, y, z;
    
    v3 operator-(v3 b)
    {
        return v3(
            x - b.x,
            y - b.y,
            z - b.z
	    );
    }
    
    v3 operator+(v3 b)
    {
        return v3(
            x + b.x,
            y + b.y,
            z + b.z
        );
    }
    
    v3 operator/(float scalar)
    {
        return v3(
            x / scalar,
            y / scalar,
            z / scalar
	    );
    }
    
    v3 operator*(float scalar)
    {
        return v3(
            x * scalar,
            y * scalar,
            z * scalar
	    );
    }
    
    v3 operator*(v3 rhs)
    {
        return v3(x*rhs.x, y*rhs.y, z*rhs.z);
    }
};

v3 operator-(v3 & v);
v3 operator*(float scalar, v3 v);
v3 cross(v3 a, v3 b);
float dot(v3 a, v3 b);
float length(v3 v);
v3 normalize(v3 a);
v3 clamp_v3(v3 a, float c);

v3 operator-(v3 & v)
{
    return v3( 
        -v.x,
        -v.y,
        -v.z
	);
}

v3 operator*(float scalar, v3 v)
{
    return v3(
        v.x * scalar,
        v.y * scalar,
        v.z * scalar
	);
}

v3 cross(v3 a, v3 b)
{
    return v3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
	);
}

float dot(v3 a, v3 b)
{
    return 
        a.x * b.x +
        a.y * b.y +
        a.z * b.z;
}

float length(v3 v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z);
}

v3 normalize(v3 a)
{
    return a / length(a);
}

v3 clamp_v3(v3 a, float c)
{
    return v3(
        a.x > c ? c : a.x,
        a.y > c ? c : a.y,
        a.z > c ? c : a.z
	);
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
    Hitable() {
	geometry = SPHERE;
	material = 0;
    }
    
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

#endif
