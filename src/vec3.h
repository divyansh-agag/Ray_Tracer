#ifndef VEC3_H
#define VEC3_H
#include <math.h>
struct vec3 {
    double x,y,z;
    vec3() = default;
    vec3(double x, double y, double z): x(x), y(y), z(z) {}

    vec3 operator+(const vec3 &other) const{
        return {x+other.x, y+other.y, z+other.z};
    }
    vec3 operator-(const vec3 &other) const{
        return {x-other.x, y-other.y, z-other.z};
    }
    vec3 operator*(const double scaler) const{
        return {x*scaler, y*scaler, z*scaler};
    }

};

double length(vec3 vec){
    double length_sq = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
    double length = sqrt(length_sq);
    return length;
}

vec3 unit_vector(vec3 vec){
    // vec length can be zero causing division by 0 fix...
    auto len = length(vec);
    return {vec.x /len, vec.y/len, vec.z/len};
}


double dot_product(vec3 a, vec3 b){
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

vec3 sub(vec3 a, vec3 b){
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}

vec3 add(vec3 a, vec3 b){
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}

vec3 cross_product(vec3 a, vec3 b){ //does NOT return unit vector convert if needed
    return {
            (a.y*b.z - a.z*b.y),
            (a.z*b.x - a.x*b.z),
            (a.x*b.y - b.x*a.y)};
}
vec3 multiply(vec3 a, vec3 b){
    return {
        a.x*b.x,
        a.y*b.y,
        a.z*b.z
    };
}

bool cmp(vec3 a, vec3 b){
    if(a.x == b.x && a.y == b.y && a.z == b.z)return 1;
    return 0;
}

#endif
