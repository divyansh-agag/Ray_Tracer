//lambertinian distribution try...
#include <bits/stdc++.h>

using namespace std;
#define NUM_OBJS 5
#define sample_per_pixel 500
#define MAX_BOUNCE_LIMIT 50//max bounce limit for any ray...
#define Height 512.0
#define Width  512.0
#define LAMBERTIAN 1
#define METAL 2
#define GLASS 3
#define MEDIUM_REFACTIVE_INDEX 1
#define pi 3.14


struct vec3 {
    double x,y,z;
};
struct Ray{
    vec3 ray_dirc;
    vec3 ray_origin;//point of origin
};

struct Sphere{
    int material;
    double fuzz;
    double r_index;
    vec3 center = {0,0,0};
    double radius = 0;
    vec3 clr = {0,0,0};//in 0.0 to 1.0.... 
};
struct Hit_info{ // struct returned by ray_clr_helper
    bool hit;
    vec3 color;
    vec3 hit_point;
    vec3 normal;
    int material; // 1 -> lambertian, 2 -> metals,  
    double fuzz;
    double r_index;
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

vec3 multiply_const(vec3 vec, double cons){
    return {vec.x*cons, vec.y*cons, vec.z*cons};
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

bool cmp(vec3 a, vec3 b){
    if(a.x == b.x && a.y == b.y && a.z == b.z)return 1;
    return 0;
}
double random_double(double minn, double maxx) {
    //genrate a random double from [minn, maxx]
    static thread_local std::mt19937 generator(1337); 
    std::uniform_real_distribution<double> distribution(minn, maxx);
    return distribution(generator);
}
vec3 genrate_random_dirc(){
    int limit = 10;
    for(int i = 0;i < limit;i++){
        double x = random_double(-1, 1);
        double y = random_double(-1, 1);
        double z = random_double(-1, 1);
        if(abs(x)<1e-60 || abs(y)<1e-60 || abs(z)<1e-60) continue;
        if(x*x + y*y + z*z <= 1) return unit_vector({x,y,z});
    }
    return unit_vector({0.5, 0.5, 0.5});

}

vec3 random_vector_hemisphere(vec3 normal){
    //returns a random vector inside the hemisphere which has  ->  normal.vec >= 0 
    vec3 random_dirc = genrate_random_dirc();
    if(dot_product(normal, random_dirc) >= 0) return random_dirc;
    else return multiply_const(random_dirc, -1);
}


pair<bool, pair<vec3, vec3>> hit_sphere(Sphere s, Ray ray){
    // returns bool for hit / not hit and if hit 2 points where ray hits the sphere..

    auto b = -2*dot_product(ray.ray_dirc, sub(s.center , ray.ray_origin));
    auto c = dot_product(sub(s.center, ray.ray_origin), sub(s.center, ray.ray_origin)) - s.radius*s.radius;
    
    auto disc = b*b - 4*c;
    if(disc>=0){
        auto disc_root = sqrt(disc);
        double t1 = (-b - disc_root)/2;
        double t2 = (-b + disc_root)/2;
        if(t1<0 && t2<0)return {false, {}}; // check so that only t>0 considered so that we only view obj in front of camera..


        vec3 point1 = add(ray.ray_origin, multiply_const(ray.ray_dirc, t1));
        vec3 point2 = add(ray.ray_origin, multiply_const(ray.ray_dirc, t2));

        if(t1 > 0.001) return {true, {point1, point1}};
        if(t2 > 0.001) return {true, {point2, point2}};
        
    }
    return {false, {}};
}

Hit_info ray_color_helper(Sphere Obj[], Ray ray){
    //handels the single ray for many objects..
    
    //returns clr in 0.0 to 1.0 range and the normal vector where ray hits and also the point where ray hits the object ...

    //normalize ray_dirc
    ray.ray_dirc = unit_vector(ray.ray_dirc);
    double a = 0.5*(ray.ray_dirc.y + 1.0);
    vec3 default_clr = add(multiply_const({1.0, 1.0, 1.0}, (1.0-a)), multiply_const({0.5, 0.7, 1.0}, a));

    // want to make this so that when ever ray hits a object it takes some color of that object 
    // and 

    double min_dist = 1e9;
    vec3 color, hit_point, unit_normal_vec;
    int material = -1;
    double fuzz;
    double r_index;
    for(int i=0;i<NUM_OBJS;i++){
        auto hit_info = hit_sphere(Obj[i], ray);
        if(hit_info.first){
            auto dist_vec1 = length(sub(hit_info.second.first, ray.ray_origin));
            auto dist_vec2 = length(sub(hit_info.second.second, ray.ray_origin));
            
            //if(min(dist_vec1, dist_vec2) < 0.0001)continue; // here min_dist > 0.0001 is added to fix the shadow acne problem...
            
            if(min(dist_vec1, dist_vec2) < min_dist){

                color = Obj[i].clr;
                fuzz = Obj[i].fuzz;
                material = Obj[i].material;
                r_index = Obj[i].r_index;
                if(dist_vec1 <= dist_vec2) hit_point = hit_info.second.first;
                else hit_point = hit_info.second.second;

                unit_normal_vec = unit_vector(sub(hit_point, Obj[i].center));
                min_dist = min(dist_vec1, dist_vec2);
            }
            
            //return {max(0.0, unit_normal_vec.x), max(0.0, unit_normal_vec.y), max(0.0, unit_normal_vec.z*0.5)};
            
        }
    }
    struct Hit_info hit_info;
    hit_info.color = color;
    hit_info.normal = unit_normal_vec;
    hit_info.hit_point = hit_point; 
    hit_info.material = material;
    hit_info.fuzz = fuzz;
    hit_info.r_index = r_index;
    if(min_dist != 1e9){
        hit_info.hit = true;
        return hit_info;   
    }
    hit_info.hit = false;
    hit_info.color = default_clr;

    return hit_info;//check here.. (default values ?)
}

void reflect(struct Ray &ray, struct Hit_info &hit_info, bool tir){
    vec3 normal = hit_info.normal;
    if(tir){
        normal = multiply_const(normal, -1);
    }
    vec3 N_star = multiply_const(normal, dot_product(multiply_const(ray.ray_dirc, -1), normal));
    vec3 fuzz_vector = multiply_const(genrate_random_dirc(), hit_info.fuzz);
    ray.ray_dirc = unit_vector(add(add(ray.ray_dirc, multiply_const(N_star, 2)), fuzz_vector));
}

vec3 ray_color(Sphere Obj[], Ray ray, int depth, double curr_ref_index){// will call this fxn for lambertian materials..
    //this will return the color for the ray 
    depth++;
    if(depth > MAX_BOUNCE_LIMIT) return {1,0,0};//can work on this part..
    Hit_info hit_info = ray_color_helper(Obj, ray);
    
    if(!hit_info.hit) {
        return hit_info.color; //if ray  misses every object returns the color of the sky
    }
    
    if(hit_info.material == LAMBERTIAN){
        ray.ray_dirc = unit_vector(add(hit_info.normal, genrate_random_dirc())); // lambertian distribution
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
            hit_info.normal = multiply_const(hit_info.normal, -1);
        }

        double cos_incident_normal = dot_product(ray.ray_dirc, multiply_const(hit_info.normal, -1));
        double sin_incident_normal = sqrt(1 - cos_incident_normal*cos_incident_normal);
        double sin_refracted_normal = (curr_ref_index * sin_incident_normal)/next_r_index;
        double cos_refracted_normal = sqrt(1 - sin_refracted_normal * sin_refracted_normal);
    
        if(sin_refracted_normal > 1){
            //TIR
            reflect(ray, hit_info, true);
        }else{
            vec3 a = multiply_const(hit_info.normal, -cos_refracted_normal);
            vec3 b = multiply_const(unit_vector(add(hit_info.normal, multiply_const(ray.ray_dirc, 1/cos_incident_normal))), sin_refracted_normal);
            
            vec3 refracted_ray = add(a,b);
            ray.ray_dirc = refracted_ray;
            
        }

        
    }       
    // Nudge origin forward to prevent self-intersection
    ray.ray_origin = add(hit_info.hit_point, multiply_const(ray.ray_dirc, 0.001));
    vec3 incoming_light = ray_color(Obj, ray, depth, curr_ref_index);

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

vector<vec3> image(Width * Height);
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

    double focal_length = 1.0;
    double fov = pi/2;// horizontal fov
    
    double viewport_Width = 2 * tan(fov/2) * focal_length;
    double viewport_Height = (Height/Width)*viewport_Width;
    
    double viewport_delw = viewport_Width/Width;
    double viewport_delh = viewport_Height / Height;
    vec3 camera_pos = {0,0,0};
    
    Sphere sphere1 ={
        //{x,y,z} -> center..
        LAMBERTIAN,0,2,
        {0,0,-2}, 
        0.5,
        {0.8,0,0}
    };
    Sphere sphere2 ={
        //{x,y,z} -> center..
        GLASS,0,1.50,
        {-1,0,-2}, 
        0.5,
        {1,1,1}
    };
    Sphere sphere3 ={
        //{x,y,z} -> center..
        METAL,0.9,1,
        {1,0,-2}, 
        0.5,
        {0.8,0.6,0.2}
    };
    Sphere sphere4 ={
        //{x,y,z} -> center..
        LAMBERTIAN,0,1,
        {0, -100.5, -4}, 
        100,    
        {0.4,0.4,0.4}
    };
    Sphere sphere5 ={
        //{x,y,z} -> center..
        GLASS,0,1/1.50,
        {-1,0,-2}, 
        0.4,
        {1,1,1}
    };
    Sphere Objects[NUM_OBJS];
    Objects[0] = sphere1;
    Objects[1] = sphere2;
    Objects[2] = sphere3;
    Objects[3] = sphere4;
    Objects[4] = sphere5;
    

    vec3 vup = {0,1,0};
    vec3 lookat = {0,-1,0};
    //v vec
    double scale_factor_vup = 1/dot_product(vup, multiply_const(lookat, -1));
    vec3 v = add(multiply_const(vup, scale_factor_vup), lookat);
    vec3 viewport_top_left = {camera_pos.x - viewport_Width/2, camera_pos.y+viewport_Height/2, -focal_length};
    //u vec
    vec3 u = unit_vector(cross_product(lookat, vup));
    
    vec3 pixel00_pos = add(sub(viewport_top_left, {0,viewport_delh/2,0}), {viewport_delw/2,0,0});

    #pragma omp parallel for schedule(dynamic)
    for(int i=0;i<(int)Height;i++){
        for(int j=0;j<(int)Width;j++){
            vec3 pixelij_pos = add(pixel00_pos, {j*viewport_delw, -i*viewport_delh, 0});
            vec3 pixelij_clr = {0,0,0};

            for(int sample = 0;sample<sample_per_pixel; sample++){// Anti-aliasing..
                vec3 pixel_cords = {
                    pixelij_pos.x + random_double(-0.5, 0.5)*viewport_delw,
                    pixelij_pos.y + random_double(-0.5, 0.5)*viewport_delh,
                    pixelij_pos.z
                };
                vec3 ray_ij_dirc = unit_vector(sub(pixel_cords, camera_pos));
                int depth = 0;
                bool debug = false;
                vec3 clr = ray_color(Objects, {ray_ij_dirc, camera_pos}, depth, MEDIUM_REFACTIVE_INDEX);

                pixelij_clr = add(pixelij_clr, clr);
                
            }
            double sample_inv_const = 1.0/sample_per_pixel;
            pixelij_clr = multiply_const(pixelij_clr, sample_inv_const);
            
            image[i *Width + j] = pixelij_clr;
            
        }

        
    }
    for(const auto& clr : image) {
        print_color(clr);
    }

    return 0;
}

//TODO: Schlick Approximation for GLASS rendering..