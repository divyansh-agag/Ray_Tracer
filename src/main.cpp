//lambertinian distribution try...
#include <bits/stdc++.h>
#define STB_IMAGE_IMPLEMENTATION

#include "vec3.h"
#include "random.h"
#include "texture.h"
#include "ray.h"
#include "hittable.h"


using namespace std;
#define sample_per_pixel 500
#define MAX_BOUNCE_LIMIT 50//max bounce limit for any ray...
#define Height 512.0
#define Width  512.0

/* --- Material Types --- */
#define LAMBERTIAN 1 
#define METAL 2 
#define GLASS 3 
/* --- Material Types --- */



#define MEDIUM_REFACTIVE_INDEX 1
#define pi 3.14
#define shutter_time 0.25


vector<vec3> image(Width * Height);
std::chrono::high_resolution_clock::time_point starttime; 

Hit_info ray_color_helper(vector<shared_ptr<Hittable>> &Obj, Ray ray, double ray_time){
    //handels the single ray for many objects..
    
    //returns clr in 0.0 to 1.0 range and the normal vector where ray hits and also the point where ray hits the object ...

    //normalize ray_dirc
    ray.ray_dirc = unit_vector(ray.ray_dirc);
    double a = 0.5*(ray.ray_dirc.y + 1.0);
    vec3 default_clr = vec3{1.0, 1.0, 1.0} * (1.0-a) + vec3{0.5, 0.7, 1.0} * a;

    // want to make this so that when ever ray hits a object it takes some color of that object 
    // and 

    double min_dist = 1e9;
    vec3 color, hit_point, unit_normal_vec;

    struct Hit_info hit_info;
    hit_info.hit = false;
    hit_info.dist = 1e9;
    hit_info.color = default_clr;
    for(auto obj: Obj){
        obj->hit(ray, hit_info, ray_time);
    }
    return hit_info;//check here.. (default values ?)
}

void reflect(struct Ray &ray, struct Hit_info &hit_info, bool tir){
    vec3 normal = hit_info.normal;
    if(tir){
        normal = (normal * -1);
    }
    vec3 N_star = (normal * dot_product((ray.ray_dirc * -1), normal));
    vec3 fuzz_vector = (genrate_random_dirc() * hit_info.fuzz);
    ray.ray_dirc = unit_vector(ray.ray_dirc + (N_star * 2) + fuzz_vector);
}

vec3 ray_color(vector<shared_ptr<Hittable>> &Obj, Ray ray, int depth, double curr_ref_index, double ray_time){// will call this fxn for lambertian materials..
    //this will return the color for the ray 

    depth++;
    if(depth > MAX_BOUNCE_LIMIT) return {1,0,0};//can work on this part..
    Hit_info hit_info = ray_color_helper(Obj, ray, ray_time);
    
    if(!hit_info.hit) {
        return hit_info.color; //if ray  misses every object returns the color of the sky
    }
    
    if(hit_info.material == LAMBERTIAN){
        ray.ray_dirc = unit_vector(hit_info.normal + genrate_random_dirc()); // lambertian distribution
    }
    else if(hit_info.material == METAL){
        //metals..
        
        reflect(ray, hit_info, false);
    }
    else if(hit_info.material == GLASS){
        bool is_entering = false;
        if(dot_product(ray.ray_dirc, hit_info.normal) < 0)is_entering = true;

        double next_r_index = MEDIUM_REFACTIVE_INDEX;
        if(is_entering) next_r_index = hit_info.r_index;
        else{
            hit_info.normal = (hit_info.normal * -1);
        }

        double cos_incident_normal = dot_product(ray.ray_dirc, (hit_info.normal * -1));
        double sin_incident_normal = sqrt(1 - cos_incident_normal*cos_incident_normal);
        double sin_refracted_normal = (curr_ref_index * sin_incident_normal)/next_r_index;
        double cos_refracted_normal = sqrt(1 - sin_refracted_normal * sin_refracted_normal);
    
        if(sin_refracted_normal > 1){
            //TIR
            reflect(ray, hit_info, true);
        }else{
            vec3 a = (hit_info.normal * -cos_refracted_normal);
            vec3 b = (unit_vector(hit_info.normal + (ray.ray_dirc * (1/cos_incident_normal))) * sin_refracted_normal);
            
            vec3 refracted_ray = a + b;
            ray.ray_dirc = refracted_ray;
            
        }

        
    }       
    // Nudge origin forward to prevent self-intersection
    ray.ray_origin = hit_info.hit_point + (ray.ray_dirc * 0.001);
    vec3 incoming_light = ray_color(Obj, ray, depth, curr_ref_index, ray_time);

    //material albedo * incoming light
    return {
            incoming_light.x * hit_info.color.x,
            incoming_light.y * hit_info.color.y,
            incoming_light.z * hit_info.color.z
    }; 
}

void print_color(vec3 clr){
    //takes color in noramal 0 to 1.0 format
    int ri = (int)(255.99 * clr.x);
    int gi = (int)(255.99 * clr.y);
    int bi = (int)(255.99 * clr.z);       
    cout<<ri<<" "<<gi<<" "<<bi<<"\n";
}


void Scene1(vector<shared_ptr<Hittable>>& Objects) {
    // Ground sphere
    Objects.push_back(
        make_shared<Sphere>(
            LAMBERTIAN,
            0.0,
            0.0,
            vec3(0, -1000, 0),
            1000,
            Texture_info(2, vec3(0.7, 0.7, 0.7), vec3(0, 1, 0), 0.5),
            vec3(0, 0, 0)
        )
    );

    // Small random spheres
    for (int a = -11; a <= 11; a++) {
        for (int b = -11; b <= 11; b++) {
            double choose_mat = random_double(0.0, 1.0);

            vec3 center(
                a + 0.9 * random_double(0.0, 1.0),
                0.2,
                b + 0.9 * random_double(0.0, 1.0)
            );

            vec3 reference(4, 0.2, 0);

            if (length(center - reference) < 0.9)
                continue;

            if (choose_mat < 0.8) {
                // Diffuse material
                Objects.push_back(
                    make_shared<Sphere>(
                        LAMBERTIAN,
                        0.0,
                        0.0,
                        center,
                        0.2,
                        Texture_info(
                            1,
                            multiply(
                                random_color(0.5, 1.0),
                                random_color(0.2, 1.0)
                            )
                        ),
                        vec3(0, 1, 0)
                    )
                );
            }
            else if (choose_mat < 0.95) {
                // Metal material
                Objects.push_back(
                    make_shared<Sphere>(
                        METAL,
                        random_double(0.0, 0.5),
                        0.0,
                        center,
                        0.2,
                        Texture_info(
                            1,
                            random_color(0.5, 1.0)
                        ),
                        vec3(0, 0, 0)
                    )
                );
            }
            else {
                // Glass material
                Objects.push_back(
                    make_shared<Sphere>(
                        GLASS,
                        0.0,
                        1.5,
                        center,
                        0.2,
                        Texture_info(
                            1,
                            vec3(1.0, 1.0, 1.0)
                        ),
                        vec3(0, 0, 0)
                    )
                );
            }
        }
    }

    // Three large spheres

    // Glass sphere
    Objects.push_back(
        make_shared<Sphere>(
            GLASS,
            0.0,
            1.5,
            vec3(0, 1, 0),
            1.0,
            Texture_info(1, vec3(1.0, 1.0, 1.0)),
            vec3(0, 0, 0)
        )
    );

    // Diffuse sphere
    Objects.push_back(
        make_shared<Sphere>(
            LAMBERTIAN,
            0.0,
            0.0,
            vec3(-4, 1, 0),
            1.0,
            Texture_info(1, vec3(0.4, 0.2, 0.1)),
            vec3(0, 0, 0)
        )
    );

    // Metal sphere
    Objects.push_back(
        make_shared<Sphere>(
            METAL,
            0.0,
            0.0,
            vec3(4, 1, 0),
            1.0,
            Texture_info(1, vec3(0.7, 0.6, 0.5)),
            vec3(0, 0, 0)
        )
    );
    /*
    CAMERA SETUP FOR THIS SCENE
        vec3 camera_pos =  {10,2,2};
    vec3 vup = {0,1,0}; // MUST BE UNIT VEC
    vec3 lookat = unit_vector({-1,0,-1}); // MUST BE UNIT VEC
    */
}

void Scene2(vector<shared_ptr<Hittable>> &Objects){
    Objects.push_back(
        make_shared<Sphere>(
            LAMBERTIAN,
            0.0, 0.0, vec3(0,-1000,-2),
            1000,
            Texture_info(RANDOM_TEXTURE, 256, 0.2),
            vec3(0, 0, 0)
        )
    );
    Objects.push_back(
        make_shared<Sphere>(
            LAMBERTIAN,
            0.0, 0.0, vec3(0,0.25,-2),
            0.25,
            Texture_info(RANDOM_TEXTURE, 256, 0.1),
            vec3(0, 0, 0)   
        )
    );
}

void Scene3(vector<shared_ptr<Hittable>> &Objects){

    // Materials / colors
    Texture_info left_red(1, vec3(1.0, 0.2, 0.2));
    Texture_info back_green(1, vec3(0.2, 1.0, 0.2));
    Texture_info right_blue(1, vec3(0.2, 0.2, 1.0));
    Texture_info upper_orange(1, vec3(0.8, 0.8, 0.8));
    Texture_info lower_teal(1, vec3(0.2, 0.8, 0.8));

    // Five quads from the book

    // Left: red
    auto left = make_shared<Quad>(
        vec3(-3, -2, 5),
        vec3(0, 0, -4),
        vec3(0, 4, 0),
        LAMBERTIAN, 0 
    );
    left->texture_info = left_red;
    Objects.push_back(left);

    // Back: green
    auto back = make_shared<Quad>(
        vec3(-2, -2, 0),
        vec3(4, 0, 0),
        vec3(0, 4, 0),
        LAMBERTIAN, 0
    );
    back->texture_info = back_green;
    Objects.push_back(back);

    // Right: blue
    auto right = make_shared<Quad>(
        vec3(3, -2, 1),
        vec3(0, 0, 4),
        vec3(0, 4, 0),
        LAMBERTIAN, 0
    );
    right->texture_info = right_blue;
    Objects.push_back(right);

    // Upper: orange
    auto upper = make_shared<Quad>(
        vec3(-2, 3, 1),
        vec3(4, 0, 0),
        vec3(0, 0, 4),
        METAL, 0
    );
    upper->texture_info = upper_orange;
    Objects.push_back(upper);

    // Lower: teal
    auto lower = make_shared<Quad>(
        vec3(-2, -3, 5),
        vec3(4, 0, 0),
        vec3(0, 0, -4),
        LAMBERTIAN, 0
    );
    lower->texture_info = lower_teal;
    Objects.push_back(lower);

}
void StartTimer(){
    // --- START TIMER ---
    starttime = chrono::high_resolution_clock::now();
    cerr << "Rendering started...\n";
}
void StopTimer(){
    // --- STOP TIMER ---
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - starttime;

    int minutes = static_cast<int>(elapsed.count()) / 60;
    double seconds = fmod(elapsed.count(), 60.0);

    cerr << "\nDone!\n";
    cerr << "Render time: " << elapsed.count() << "s (" 
         << minutes << "m " << fixed << setprecision(2) << seconds << "s)\n";
}
int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    //-ve z viewport dirc
    // up -> +y
    // in dic of Width +x(right handed system)
    cout << "P3\n"
        << Width << " " << Height << "\n"
        << "255\n";
    
    
    vector<shared_ptr<Hittable>> Objects;

    Scene3(Objects);
    
    double focal_length = 1.0;
    double fov = pi/2;// horizontal fov
    
    double viewport_Width = 2 * tan(fov/2) * focal_length;
    double viewport_Height = (Height/Width)*viewport_Width;
    
    double viewport_delw = viewport_Width/Width;
    double viewport_delh = viewport_Height / Height;

    vec3 camera_pos =  {0,0,9};
    vec3 vup = {0,1,0}; // MUST BE UNIT VEC
    vec3 lookat = unit_vector({0,0,-1}); // MUST BE UNIT VEC

    vec3 u = unit_vector(cross_product(lookat, vup));
    vec3 v = unit_vector(cross_product(u, lookat));
    vec3 viewport_delw_vec = (u * viewport_delw);
    vec3 viewport_delh_vec = (v * viewport_delh);


    vec3 viewport_top_left = camera_pos + (lookat * focal_length) + (v * (viewport_Height/2)) + (u * (-viewport_Width/2));
    //{camera_pos.x - viewport_Width/2, camera_pos.y+viewport_Height/2, -focal_length};
    vec3 pixel00_pos = viewport_top_left + (viewport_delw_vec * 0.5) + (viewport_delh_vec * -0.5);
    


    StartTimer();
    #pragma omp parallel for schedule(dynamic)
    for(int i=0;i<(int)Height;i++){
        for(int j=0;j<(int)Width;j++){
            vec3 pixelij_pos = pixel00_pos + (viewport_delw_vec * j) + (viewport_delh_vec * -i);
            vec3 pixelij_clr = {0,0,0};

            for(int sample = 0;sample<sample_per_pixel; sample++){// Anti-aliasing..
                double ray_time = random_double(0.0, shutter_time);

                vec3 pixel_cords = pixelij_pos + (viewport_delw_vec * random_double(-0.5, 0.5)) + (viewport_delh_vec * random_double(-0.5, 0.5));
                vec3 ray_ij_dirc = unit_vector(pixel_cords - camera_pos);
                int depth = 0;
                vec3 clr = ray_color(Objects, {ray_ij_dirc, camera_pos}, depth, MEDIUM_REFACTIVE_INDEX, ray_time);

                pixelij_clr = pixelij_clr + clr;
                
            }
            double sample_inv_const = 1.0/sample_per_pixel;
            pixelij_clr = (pixelij_clr * sample_inv_const);
            
            image[i * Width + j] = pixelij_clr;
            
        }
        
    }
    StopTimer();

    for(const auto& clr : image) {
        print_color(clr);
    }

    return 0;
}

