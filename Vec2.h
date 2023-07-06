#ifndef VEC2_H_
#define VEC2_H_

// ------------------------------------------------

typedef struct Vec2 {
  float x;
  float y;
} Vec2;

// ------------------------------------------------
Vec2* initialise_vec2();
Vec2* scalar_mult(float scalar, Vec2* vect);
Vec2* vect_sum(Vec2* vect1, Vec2* vect2);
Vec2* vect_sub(Vec2* vect1, Vec2* vect2);
float scalar_product(Vec2* vect1, Vec2* vect2);
Vec2* normalisation(Vec2* vect);
float norme(Vec2* vect);

// ------------------------------------------------

#endif