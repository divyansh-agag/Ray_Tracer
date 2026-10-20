#ifndef RANDOM_H
#define RANDOM_H

#include <bits/stdc++.h>
double random_double(double minn, double maxx) {
    //genrate a random double from [minn, maxx]
    static thread_local std::mt19937 generator(1337); 
    std::uniform_real_distribution<double> distribution(minn, maxx);
    return distribution(generator);
}
int rand_int(int min, int max) {
    return static_cast<int>(random_double(min, max + 1));
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
vec3 random_color(double minn, double maxx){
    return{
        random_double(minn, maxx),
        random_double(minn, maxx),
        random_double(minn, maxx)
    };
}


#endif
