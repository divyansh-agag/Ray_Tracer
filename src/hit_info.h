
#ifndef HIT_INFO_H
#define HIT_INFO_H
#include "vec3.h"
struct Hit_info{ // struct returned by ray_clr_helper
    bool hit;
    double dist;
    int obj_type; // sphere or quad
    vec3 color;
    vec3 hit_point;
    vec3 normal;
    int material; // 1 -> lambertian, 2 -> metals,  
    double fuzz;
    double r_index;
    double u, v;
    double emmisivity;
};

#endif