
#ifndef HITTABLE_H
#define HITTABLE_H
#include "texture.h"
#include "ray.h"


#define SPHERE_OBJ 1
#define QUAD_OBJ 2
struct Hittable {
    virtual pair<bool, vec3> hit(struct Ray& r, struct Hit_info &hit_info, double ray_time) const = 0;

    virtual ~Hittable() = default;
};

struct Sphere : Hittable {
    int material;
    double fuzz;
    double r_index;
    vec3 st_center = {0,0,0};
    double radius = 0;
    struct Texture_info texture_info; //for textures can use a fuction in place of this
    vec3 vel = {0,0,0};
    double emmisivity = 0;

    Sphere() = default;
    Sphere(int mat, double f, double ri, vec3 center, double r, Texture_info tex, vec3 v = {0, 0, 0}, double emmisivity = 0)
        : material(mat), fuzz(f), r_index(ri), st_center(center), radius(r), texture_info(tex), vel(v), emmisivity(emmisivity) {}


    pair<bool, vec3> hit(struct Ray& ray, struct Hit_info &hit_info, double ray_time) const override {
        // returns bool for hit / not hit and if hit 2 points where ray hits the sphere..
        vec3 curr_pos = st_center + vel * ray_time;
        auto b = -2*dot_product(ray.ray_dirc, curr_pos - ray.ray_origin);
        auto c = dot_product(curr_pos - ray.ray_origin, curr_pos - ray.ray_origin) - (radius*radius);
        
        auto disc = b*b - 4*c;
        if(disc>=0){
            auto disc_root = sqrt(disc);
            double t1 = (-b - disc_root)/2;
            double t2 = (-b + disc_root)/2;
            if(t1<0 && t2<0)return {false, {}}; // check so that only t>0 considered so that we only view obj in front of camera..


            vec3 point1 = ray.ray_origin + ray.ray_dirc * t1;
            vec3 point2 = ray.ray_origin + ray.ray_dirc * t2;

            if(t1 > 0.001) {
                hit_info.hit = true;
                if(t1 < hit_info.dist){
                    hit_info.dist = t1;
                    hit_info.obj_type = SPHERE_OBJ;
                    
                    hit_info.fuzz = fuzz;
                    hit_info.material = material;
                    hit_info.r_index = r_index;
                    hit_info.hit_point = point1;
                    hit_info.normal = unit_vector(hit_info.hit_point - curr_pos);
                    get_u_v(hit_info, radius);
                    hit_info.emmisivity = emmisivity;
                    hit_info.color = texture_info.give_color(hit_info);
                }
                return {true, point1};
            }
            if(t2 > 0.001) {
                hit_info.hit = true;
                if(t2 < hit_info.dist){
                    hit_info.dist = t2;
                    hit_info.obj_type = SPHERE_OBJ;
                    
                    hit_info.fuzz = fuzz;
                    hit_info.material = material;
                    hit_info.r_index = r_index;
                    hit_info.hit_point = point2;
                    hit_info.normal = unit_vector(hit_info.hit_point - curr_pos);
                    get_u_v(hit_info, radius);
                    hit_info.emmisivity = emmisivity;
                    hit_info.color = texture_info.give_color(hit_info);
                }
                return {true, point2};
            }
            
        }

        return {false, {}};
    }
    void get_u_v(struct Hit_info &hit_info, double radius)const {
        // returns {u,v} cords for a given hit_point for an Image_Texture
        double phi = atan2(hit_info.normal.x, hit_info.normal.z) + pi;
        double theta = acos(hit_info.normal.y);

        hit_info.u = phi/(2*pi);
        hit_info.v = theta / pi;

    }

};

struct Quad : Hittable {
    vec3 Q;
    vec3 u;
    vec3 v;
    vec3 N;
    int material;
    double fuzz;  
    struct Texture_info texture_info;
    double emmisivity = 0;
    Quad(vec3 Q, vec3 u, vec3 v, int material, double fuzz, double emmisivity, Texture_info texture_info = {}) : Q(Q), u(u), v(v), material(material), fuzz(fuzz), emmisivity(emmisivity), texture_info(texture_info){
        N = cross_product(u, v);
    }
    pair<bool, vec3> hit(struct Ray& ray, struct Hit_info &hit_info, double ray_time) const override {
        if(abs(dot_product(ray.ray_dirc, N)) <1e-10)return {false, {}};
        double t = dot_product(Q - ray.ray_origin, N) / dot_product(ray.ray_dirc, N);
        if(t < 0)return {false, {}};
        
        if(hit_inside(ray, t)) {
            hit_info.hit = true;
            if(t < hit_info.dist){
                hit_info.obj_type = QUAD_OBJ;
                hit_info.dist = t;
                hit_info.hit_point = ray.ray_origin + ray.ray_dirc * t;
                hit_info.normal = unit_vector(N);
                hit_info.material = material;
                hit_info.fuzz = fuzz;
                hit_info.emmisivity = emmisivity;
                hit_info.color = texture_info.give_color(hit_info);
            }
            return {true, ray.ray_origin + ray.ray_dirc * t};
        }
        return {false, {}};
    }
    bool hit_inside(struct Ray &ray, double t)const {
        vec3 A = ray.ray_origin + ray.ray_dirc * t;
        vec3 p = A - Q;
        double b = dot_product(N, cross_product(p, v)) / dot_product(N, cross_product(u, v));
        double a = dot_product(N, cross_product(p, u)) / dot_product(N, cross_product(v, u));

        if(a >= 0 && a <= 1 && b >= 0 && b <= 1)return true;
        return false;
    }
};

#endif