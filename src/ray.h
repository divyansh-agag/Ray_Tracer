
#ifndef RAY_H
#define RAY_H
#include "vec3.h"
struct Ray{
    vec3 ray_dirc;
    vec3 ray_origin;//point of origin

    Ray(vec3 ray_dirc, vec3 ray_origin): ray_dirc(ray_dirc), ray_origin(ray_origin) {}
};

#endif