#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_

// ------------------------------------------------

typedef struct SphereCollider {
  Vec2 center;
  float radius;
} SphereCollider;

typedef struct PlanCollider {
  Vec2 point;
  Vec2 normale;
} PlanCollider;

// ------------------------------------------------

#endif