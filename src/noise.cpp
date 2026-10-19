#include <bits/stdc++.h>
using namespace std;

struct vec3 {
    double x,y,z;
    vec3() = default;
    vec3(double x, double y, double z): x(x), y(y), z(z) {}

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

void print_color(vec3 clr){
    //takes color in noramal 0 to 1.0 format
    int ri = (int)(255.99 * clr.x);
    int gi = (int)(255.99 * clr.y);
    int bi = (int)(255.99 * clr.z);       
    cout<<ri<<" "<<gi<<" "<<bi<<"\n";
}

double random_double(double minn, double maxx) {
    //genrate a random double from [minn, maxx]
    static thread_local std::mt19937 generator(1337); 
    std::uniform_real_distribution<double> distribution(minn, maxx);
    return distribution(generator);
}


int main(){
    int height = 512;
    int width = 512;
    cout << "P3\n"
        << width << " " << height << "\n"
        << "255\n";

    
    for(int i=0; i<height; i++){
        for(int j=0; j< width; j++){
            print_color(vec3(1,1,1)*random_double(0.0, 1.0));
        }
    }
    return 0;
}