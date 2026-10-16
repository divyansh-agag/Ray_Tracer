//lambertinian distribution try...
#include <bits/stdc++.h>

using namespace std;
#define sample_per_pixel 500
#define MAX_BOUNCE_LIMIT 50//max bounce limit for any ray...
#define Height 337.0
#define Width  600.0
#define LAMBERTIAN 1
#define METAL 2
#define GLASS 3
#define MEDIUM_REFACTIVE_INDEX 1
#define pi 3.14
#define shutter_time 0.25

struct vec3 {
    double x,y,z;

    vec3 operator+(const vec3 &other){
        return {x+other.x, y+other.y, z+other.z};
    }
    vec3 operator-(const vec3 &other){
        return {x-other.x, y-other.y, z-other.z};
    }
    vec3 operator*(double scaler){
        return {x*scaler, y*scaler, z*scaler};
    }

};

vector<vec3> image(Width * Height);
std::chrono::high_resolution_clock::time_point starttime; 

struct Ray{
    vec3 ray_dirc;
    vec3 ray_origin;//point of origin
};

struct Sphere{/*int material;
    double fuzz;
    double r_index;
    vec3 st_center = {0,0,0};
    vec3 curr_center = {0,0,0};
    double radius = 0;
    vec3 clr = {0,0,0};//in 0.0 to 1.0.... 
    vec3 vel = {0,0,0};
    */
    int material;
    double fuzz;
    double r_index;
    vec3 st_center = {0,0,0};
    double radius = 0;
    vec3 clr = {0,0,0};//in 0.0 to 1.0.... 
    vec3 vel = {0,0,0};
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
    else return random_dirc * -1;
}


pair<bool, vec3> hit_sphere(Sphere s, Ray ray, double ray_time){
    // returns bool for hit / not hit and if hit 2 points where ray hits the sphere..
    vec3 curr_pos = s.st_center + s.vel * ray_time;
    auto b = -2*dot_product(ray.ray_dirc, curr_pos - ray.ray_origin);
    auto c = dot_product(curr_pos - ray.ray_origin, curr_pos - ray.ray_origin) - (s.radius*s.radius);
    
    auto disc = b*b - 4*c;
    if(disc>=0){
        auto disc_root = sqrt(disc);
        double t1 = (-b - disc_root)/2;
        double t2 = (-b + disc_root)/2;
        if(t1<0 && t2<0)return {false, {}}; // check so that only t>0 considered so that we only view obj in front of camera..


        vec3 point1 = ray.ray_origin + ray.ray_dirc * t1;
        vec3 point2 = ray.ray_origin + ray.ray_dirc * t2;

        if(t1 > 0.001) return {true, point1};
        if(t2 > 0.001) return {true, point2};
        
    }
    return {false, {}};
}

Hit_info ray_color_helper(vector<Sphere> &Obj, Ray ray, double ray_time){
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
    int material = -1;
    double fuzz;
    double r_index;
    for(int i=0;i<Obj.size();i++){
        vec3 curr_pos = Obj[i].st_center + Obj[i].vel * ray_time;
        auto hit_info = hit_sphere(Obj[i], ray, ray_time);
        if(hit_info.first){
            auto curr_dist = length(hit_info.second - ray.ray_origin);
            
            if(curr_dist < min_dist){

                color = Obj[i].clr;
                fuzz = Obj[i].fuzz;
                material = Obj[i].material;
                r_index = Obj[i].r_index;
                hit_point = hit_info.second;
                unit_normal_vec = unit_vector(hit_info.second - curr_pos);
                min_dist = curr_dist;
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
        normal = (normal * -1);
    }
    vec3 N_star = (normal * dot_product((ray.ray_dirc * -1), normal));
    vec3 fuzz_vector = (genrate_random_dirc() * hit_info.fuzz);
    ray.ray_dirc = unit_vector(ray.ray_dirc + (N_star * 2) + fuzz_vector);
}

vec3 ray_color(vector<Sphere> &Obj, Ray ray, int depth, double curr_ref_index, double ray_time){// will call this fxn for lambertian materials..
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
vec3 random_color(double minn, double maxx){
    return{
        random_double(minn, maxx),
        random_double(minn, maxx),
        random_double(minn, maxx)
    };
}


void Init_Objs(vector<Sphere> &Objects){
    // Ground sphere
    Objects.push_back({
        LAMBERTIAN,
        0.0,
        0.0,
        {0, -1000, 0},
        1000,
        {0.7, 0.7, 0.7},
        {0,0,0}
    });

    // Small random spheres
    for (int a = -11; a <= 11; a++) {
        for (int b = -11; b <= 11; b++) {

            double choose_mat = random_double(0.0, 1.0);

            vec3 center = {
                a + 0.9 * random_double(0.0, 1.0),
                0.2,
                b + 0.9 * random_double(0.0, 1.0)
            };

            // Keep the small spheres away from the large sphere at (4, 0.2, 0)
            vec3 reference = {4, 0.2, 0};

            if (length(center - reference) < 0.9) {
                continue;
            }

            if (choose_mat < 0.8) {
                // Diffuse material
                Objects.push_back({
                    LAMBERTIAN,
                    0.0,
                    0.0,
                    center,
                    0.2,
                    multiply(
                        random_color(0.5, 1.0),
                        random_color(0.2, 1.0)
                    ),
                    {0,1,0}
                });

            } else if (choose_mat < 0.95) {
                // Metal material
                Objects.push_back({
                    METAL,
                    random_double(0.0, 0.5),
                    0.0,
                    center,
                    0.2,
                    random_color(0.5, 1.0),
                    {0,0,0}
                });

            } else {
                // Glass material
                Objects.push_back({
                    GLASS,
                    0.0,
                    1.5,
                    center,
                    0.2,
                    {1.0, 1.0, 1.0},
                    {0,0,0}
                });
            }
        }
    }

    // Three large spheres

    // Center: glass
    Objects.push_back({
        GLASS,
        0.0,
        1.5,
        {0, 1, 0},
        1.0,
        {1.0, 1.0, 1.0},
        {0,0,0}
    });

    // Left: diffuse
    Objects.push_back({
        LAMBERTIAN,
        0.0,
        0.0,
        {-4, 1, 0},
        1.0,
        {0.4, 0.2, 0.1},
        {0,0,0}
    });

    // Right: metal
    Objects.push_back({
        METAL,
        0.0,
        0.0,
        {4, 1, 0},
        1.0,
        {0.7, 0.6, 0.5},
        {0,0,0}
    });
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

    
    
    vector<Sphere> Objects;

    Init_Objs(Objects);
    
    double focal_length = 1.0;
    double fov = pi/6;// horizontal fov
    
    double viewport_Width = 2 * tan(fov/2) * focal_length;
    double viewport_Height = (Height/Width)*viewport_Width;
    
    double viewport_delw = viewport_Width/Width;
    double viewport_delh = viewport_Height / Height;

    vec3 camera_pos = {13,2,3};
    vec3 vup = {0,1,0}; // MUST BE UNIT VEC
    vec3 lookat = unit_vector({-3,-0.25,-1}); // MUST BE UNIT VEC

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

//TODO: Schlick Approximation for GLASS rendering
//Defocus Blur