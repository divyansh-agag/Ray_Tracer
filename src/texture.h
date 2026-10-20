
#ifndef TEXTURE_H
#define TEXTURE_H
#include <bits/stdc++.h>
#include "vec3.h"
#include "hit_info.h"
#include "external/stb_image.h"
#include "random.h"

using namespace std;

/* --- Texture Types ---*/
#define SOLID_COLOR 1
#define CHECKER_TEXTURE 2
#define IMAGE_TEXTURE 3
#define RANDOM_TEXTURE 4
/* --- Texture Types ---*/
#define pi 3.14

struct Texture_info{
    int Texture_type = SOLID_COLOR;
    vec3 clr = {0,0,0}; //solid 
    vec3 clr1 = {0,0,0}; //checker 
    vec3 clr2 = {1,1,1}; //checker
    double scale = 1.0; //checker 
    double longitudnal_phase = 0.0; // [0, 2*pi] Y-AXIS
    double latitunal_phase = 0.0; // [0, pi] X-AXIS
    const char* filename = nullptr; //image texture
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    // Random
    int num_points;
    double *rand_points = nullptr; // random points array
    int *prem_x, *prem_y, *prem_z;
    


    Texture_info() = default;

    Texture_info(int type, vec3 clr): Texture_type(type), clr(clr) {}

    Texture_info(int type, vec3 clr1, vec3 clr2, double scale): Texture_type(type), clr1(clr1), clr2(clr2), scale(scale) {}

    Texture_info(int type, const char* image_path, double latitunal_phase, double longitudnal_phase): Texture_type(type), filename(image_path), latitunal_phase(latitunal_phase), longitudnal_phase(longitudnal_phase) {
        int comps_pre_pixel = 3;
        clamp(latitunal_phase, 0.0, pi); 
        clamp(longitudnal_phase, 0.0, 2*pi);
        data = stbi_load(filename, &width, &height, &comps_pre_pixel, 3);
        //Error Handeling
    }   

    Texture_info(int type, int num_points, double scale): Texture_type(type), num_points(num_points), scale(scale) {
        rand_points = new double [num_points];
        prem_x = new int [num_points];
        prem_y = new int [num_points];
        prem_z = new int [num_points];
        for(int i=0; i<num_points; i++) {
            rand_points[i] = random_double(0,1);
            prem_x[i] = i;
            prem_y[i] = i;
            prem_z[i] = i;
        }   
        for(int i=0; i<num_points; i++){
            int pos = rand_int(0, num_points-1);
            swap(prem_x[i], prem_x[pos]);
            pos = rand_int(0, num_points-1);
            swap(prem_y[i], prem_y[pos]);
            pos = rand_int(0, num_points-1);
            swap(prem_z[i], prem_z[pos]);
        }
        
    }



    vec3 give_color(const struct Hit_info &hit_info) const {
        if(Texture_type == SOLID_COLOR){
            return clr;
        }else if(Texture_type == CHECKER_TEXTURE){
            int parity = floor(hit_info.hit_point.x * 1/scale) 
            +floor(hit_info.hit_point.y * 1/scale)
            +floor(hit_info.hit_point.z * 1/scale);
        
            if(parity % 2 == 0) return clr1;
            else return clr2;    
        }else if(Texture_type == IMAGE_TEXTURE){       
             
            int i = (int)(hit_info.u * width + longitudnal_phase*width/(2*pi))%width;
            int j = (int)(hit_info.v * height + latitunal_phase*height/(pi))%height;
            if(i >= width)i = width-1;
            if(j >= height)j = height-1;
            int bytes_per_scanline = 3 * width;

            unsigned char *pixel_data = data + bytes_per_scanline*j + i*3;

            return {
                pixel_data[0] * 1.0/255.0,
                pixel_data[1] * 1.0/255.0,
                pixel_data[2] * 1.0/255.0
            };
        }else if(Texture_type == RANDOM_TEXTURE){
            
            double x = hit_info.hit_point.x * 1/scale;
            double y = hit_info.hit_point.y * 1/scale;
            double z = hit_info.hit_point.z * 1/scale;
            
            double u = x - floor(x);
            double v = y - floor(y);
            double w = z - floor(z);
            u = u*u*(3-2*u);
            v = v*v*(3-2*v);
            w = w*w*(3-2*w);
            double c[2][2][2];
            for(int di=0;di<2;di++)
                for(int dj=0;dj<2;dj++)
                    for(int dk=0;dk<2;dk++)
                        c[di][dj][dk] = rand_points[prem_x[(int)(x+di) & 255] ^ prem_y[(int)(y+dj) & 255] ^ prem_z[(int)(z+dk) & 255]];
            return vec3(1,1,1) * interpolate(c, u, v, w);
            
        }
        return {0,0,0}; // default
    }
    double interpolate(double c[2][2][2], double u,double v,double w) const{
        double accum = 0;
        for(int i=0;i<2;i++)
            for(int j=0;j<2;j++)
                for(int k=0;k<2;k++)
                    accum += (i*u + (1-i)*(1-u))
                           * (j*v + (1-j)*(1-v))
                           * (k*w + (1-k)*(1-w))
                           * c[i][j][k];
        return accum; //note that accum will never be > 1.0 so chill out :)
    }
};

#endif