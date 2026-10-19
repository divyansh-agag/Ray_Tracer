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

#endif
