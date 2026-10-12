//Diffuse materials try..

#include <bits/stdc++.h>

using namespace std;
#define NUM_OBJS 4
#define sample_per_pixel 50
#define MAX_BOUNCE_LIMIT 10 //max bounce limit for any ray...
#define Height 512.0
#define Width  512.0
// Allocate image buffer


struct vec3 {
    double x,y,z;
};
struct Ray{
    vec3 ray_dirc;
    vec3 ray_origin;//point if origin
};

struct Sphere{
    vec3 center = {0,0,0};
    double radius = 0;
    vec3 clr = {0,0,0};//in 0.0 to 1.0.... 
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
bool cmp(vec3 a, vec3 b){
    if(a.x == b.x && a.y == b.y && a.z == b.z)return 1;
    return 0;
}
double random_double(double minn, double maxxx) {
    //genrate a random double from [minn, maxx]
    static thread_local std::mt19937 generator(1337); 
    std::uniform_real_distribution<double> distribution(minn, maxxx);
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
        double t1 = (-b + disc_root)/2;
        double t2 = (-b - disc_root)/2;
        if(t1<0 && t2<0)return {false, {}}; // check so that only t>0 considered so that we only view obj in front of camera..

        vec3 point1 = add(ray.ray_origin, multiply_const(ray.ray_dirc, t1));
        vec3 point2 = add(ray.ray_origin, multiply_const(ray.ray_dirc, t2));
        if(t1>=0 && t2<0)return {true, {point1, point1}};
        else if(t1<0 && t2>=0) return {true, {point2, point2}};

        return {true, {point1,point2}};
    
    }
    return {false, {}};
}

pair<bool, vector<vec3>> ray_color_helper(Sphere Obj[], Ray ray){
    //handels the single ray for many objects..
    
    //returns clr in 0.0 to 1.0 range and the normal vector where ray hits and also the point where ray hits the object ...

    //normalize ray_dirc
    ray.ray_dirc = unit_vector(ray.ray_dirc);
    vec3 default_clr = {0,0,0};

    // want to make this so that when ever ray hits a object it takes some color of that object 
    // and 

    double min_dist = 1e9;
    vec3 color, hit_point, unit_normal_vec;
    for(int i=0;i<NUM_OBJS;i++){
        auto hit_info = hit_sphere(Obj[i], ray);
        if(hit_info.first){
            auto dist_vec1 = length(sub(hit_info.second.first, ray.ray_origin));
            auto dist_vec2 = length(sub(hit_info.second.second, ray.ray_origin));
            
            
            if(min(dist_vec1, dist_vec2) < min_dist){

                color = Obj[i].clr;
                if(dist_vec1 <= dist_vec2) hit_point = hit_info.second.first;
                else hit_point = hit_info.second.second;

                unit_normal_vec = unit_vector(sub(hit_point, Obj[i].center));
                min_dist = min(dist_vec1, dist_vec2);
            }
            
            //return {max(0.0, unit_normal_vec.x), max(0.0, unit_normal_vec.y), max(0.0, unit_normal_vec.z*0.5)};
            
        }
    }
    if(min_dist != 1e9) return {true, {color, unit_normal_vec, hit_point}};

    return {false, {default_clr, default_clr, default_clr}};//check here..
}
vec3 ray_color(Sphere Obj[], Ray ray){
    vec3 default_clr = {0,0,0};
    vec3 curr_clr = {1,1,1};

    int hits = 0;
    vec3 clr;
    vec3 normal;
    vec3 hit_point;
    for(int i=0;i<MAX_BOUNCE_LIMIT;i++){
        auto info = ray_color_helper(Obj, ray);
        if(!info.first)break;//not hit recorded..


        clr = info.second[0];
        normal = info.second[1];
        hit_point = info.second[2];

        curr_clr.x *= clr.x;
        curr_clr.y *= clr.y;
        curr_clr.z *= clr.z;
        
        ray.ray_dirc = random_vector_hemisphere(normal);
        ray.ray_origin = hit_point;
        hits++;

    }
    if(hits == 0)return default_clr;
    return curr_clr;


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
    double viewport_Width = 2.0;
    double viewport_Height = (Height/Width)*viewport_Width;
    
    double viewport_delw = viewport_Width/Width;
    double viewport_delh = viewport_Height / Height;
    vec3 camera_pos = {0,0,0};
    
    Sphere sphere1 ={
        //{x,y,z} -> center..
        {-1,0,-2}, 
        0.5,
        {1,0,0}
    };
    Sphere sphere2 ={
        //{x,y,z} -> center..
        {1,0,-2}, 
        0.5,
        {0,1,0}
    };
    Sphere sphere3 ={
        {0,0,-2}, 
        0.5,
        {1,1,1}
        
    };
    Sphere sphere4 ={
        //{x,y,z} -> center..
        {0, -100.5, -4}, 
        100,    
        {1,1,1}
    };
    //Sphere sphere3 ={
    //    {0,1,-3}, 
    //    1,
    //    {1.0,1.0,1.0}
    //};
    Sphere Objects[5];
    Objects[0] = sphere1;
    Objects[1] = sphere2;
    Objects[2] = sphere3;
    Objects[3] = sphere4;
    //cout<<viewport_Height<<" "<<viewport_Width<<endl;
    
    
    vec3 viewport_top_left = {camera_pos.x - viewport_Width/2, camera_pos.y+viewport_Height/2, -focal_length};
    //
    vec3 pixel00_pos = add(sub(viewport_top_left, {0,viewport_delh/2,0}), {viewport_delw/2,0,0});

    #pragma omp parallel for schedule(dynamic)
    for(int i=0;i<(int)Height;i++){
        for(int j=0;j<(int)Width;j++){
            vec3 pixelij_pos = add(pixel00_pos, {j*viewport_delw, -i*viewport_delh,0});
            vec3 pixelij_clr = {0,0,0};

            for(int sample = 0;sample<sample_per_pixel; sample++){// Anti-aliasing..
                vec3 pixel_cords = {
                    pixelij_pos.x + random_double(-0.5, 0.5)*viewport_delw,
                    pixelij_pos.y + random_double(-0.5, 0.5)*viewport_delh,
                    pixelij_pos.z
                };
                vec3 ray_ij_dirc = sub(pixel_cords, camera_pos);
                vec3 clr = ray_color(Objects, {ray_ij_dirc, camera_pos});
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