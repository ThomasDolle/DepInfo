#include <stdlib.h>
#include <stdio.h>
#include "Vec2.h"
#include <math.h> 


Vec2* initialise_vec2(){
    Vec2* vect = malloc(sizeof(struct Vec2)); //malloc une fois pour toutes
    vect->x = 0;
    vect->y = 0;
    return vect;
}

Vec2* scalar_mult(float scalar, Vec2* vect){
    Vec2* multiplied_vect = initialise_vec2(); //malloc implicite
    multiplied_vect->x = scalar*vect->x;
    multiplied_vect->y = scalar*vect->y;
    return multiplied_vect;
}

Vec2* vect_sum(Vec2* vect1, Vec2* vect2){
    Vec2* summed_vect = initialise_vec2();
    summed_vect->x = vect1->x + vect2->x;
    summed_vect->y = vect1->y + vect2->y;
    return summed_vect;
}

Vec2* vect_sub(Vec2* vect1, Vec2* vect2){
    Vec2* summed_vect = initialise_vec2();
    summed_vect->x = vect1->x - vect2->x;
    summed_vect->y = vect1->y - vect2->y;
    return summed_vect;
}

float scalar_product(Vec2* vect1, Vec2* vect2){
    float result = (vect1->x * vect2->x) + (vect1->y * vect2->y);
    return result;
}

Vec2* normalisation(Vec2* vect){
    Vec2* norm_vect = initialise_vec2();
    float norm = sqrt(vect->x*vect->x + vect->y*vect->y);
    norm_vect->x = vect->x/norm;
    norm_vect->y = vect->y/norm;
    return norm_vect;
}

float norme(Vec2* vect) {
    return sqrt(scalar_product(vect,vect));
}
