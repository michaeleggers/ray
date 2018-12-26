#include <stdio.h>
#include <math.h>

#include "raytracer.h"


int main(int argc, char** argv)
{
    
    v3 normal = {0, 1, 0};
    v3 incident = {0.5, 0.5, 0};
    incident = normalize(incident);
    
    float cosni = dot(normal, incident);
    printf("cos: %f\n", cosni);
    return 0;
}

