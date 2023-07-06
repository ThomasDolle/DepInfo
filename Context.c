#include "Context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// ------------------------------------------------

Particle getParticle(Context* context, int id)
{
  return context->particles[id];
}

// ------------------------------------------------

void addParticle(Context* context, float x, float y, float radius, float mass, int draw_id)
{
    assert(context->num_particles<context->capacity_particles); // currently no resize in context
    context->particles[context->num_particles].position.x = x;
    context->particles[context->num_particles].position.y = y;
    context->particles[context->num_particles].velocity.x = 0.0F;
    context->particles[context->num_particles].velocity.y = 0.0F;
    context->particles[context->num_particles].inv_mass = 1.0F/mass;
    context->particles[context->num_particles].radius = radius;
    context->particles[context->num_particles].draw_id = draw_id;
    context->num_particles += 1;
}

// ------------------------------------------------

void setDrawId(Context* context, int sphere_id, int draw_id)
{
  context->particles[sphere_id].draw_id = draw_id;
}

// ------------------------------------------------

SphereCollider getGroundSphereCollider(Context* context, int id)
{
  return context->ground_spheres[id];
}

// ------------------------------------------------

PlanCollider getPlanCollider(Context* context)
{
  return context->plan;
}

// ------------------------------------------------

Context* initializeContext(int capacity)
{
  Context* context = malloc(sizeof(Context));

  context->num_particles = 0;
  context->capacity_particles = capacity;
  context->particles = malloc(capacity*sizeof(Particle));
  memset(context->particles,0,capacity*sizeof(Particle));

  context->num_ground_sphere = 4;
  context->ground_spheres = malloc(4*sizeof(SphereCollider));
  Vec2 p0 = {-3.0f, 5.0f};
  context->ground_spheres[0].center = p0;
  context->ground_spheres[0].radius = 1.7;
  Vec2 p1 = {3.0f, 5.0f};
  context->ground_spheres[1].center = p1;
  context->ground_spheres[1].radius = 1.7;
  Vec2 p2 = {-7.0f, -3.0f};
  context->ground_spheres[2].center = p2;
  context->ground_spheres[2].radius = 2.5;
  Vec2 p3 = {7.0f, -3.0f};
  context->ground_spheres[3].center = p3;
  context->ground_spheres[3].radius = 2.5;

  Vec2 p4={-10.0f, -8.0f};
  context->plan.point=p4;
  Vec2 p5={0.0f, 1.0f};
  context->plan.normale=p5;

  return context;
}

// ------------------------------------------------

void updatePhysicalSystem(Context* context, float dt, int num_constraint_relaxation)
{
  applyExternalForce(context, dt);
  dampVelocities(context);
  updateExpectedPosition(context, dt);
  addDynamicContactConstraints(context);
  addStaticContactConstraints(context);
 
  for(int k=0; k<num_constraint_relaxation; ++k) {
    projectConstraints(context);
  }

  updateVelocityAndPosition(context, dt);
  applyFriction(context);

  deleteContactConstraints(context);
}

// ------------------------------------------------

void applyExternalForce(Context* context, float dt)
{
  if (context->num_particles == 0) return;
  Particle *p = context->particles;
  for (int i = 0; i<context->num_particles; i++){
    (p+i)->velocity.y += - dt*(p+i)->inv_mass * 9.81;
  }
}

void dampVelocities(Context* context)
{
}

void updateExpectedPosition(Context* context, float dt)
{
  if (context->num_particles == 0) return;
  Particle *p = context->particles;
  for (int i = 0; i<context->num_particles; i++){
    (p+i)-> next_pos.x = (p+i)->position.x +dt * (p+i)->velocity.x;
    (p+i)-> next_pos.y = (p+i)->position.y +dt * (p+i)->velocity.y;
  }
}

void addDynamicContactConstraints(Context* context)
{
}

void addStaticContactConstraints(Context* context)
{ 
  if (context->num_particles == 0) return;
  Particle *p = context->particles;

  if (context->num_ground_sphere == 0) return;
  SphereCollider *s = context->ground_spheres;
 
  for (int i = 0; i<context->num_particles; i++){
/*
    // choc avec le plan avec les equations mais la boule s'arrête une fois sur deux
    Vec2* difference = vect_sub(&(p+i)->next_pos,&context->plan.point);
    if ( scalar_product(&context->plan.normale,difference)<=(p+i)->radius ) {//diff est un pointeur paln.normale est un vecteur donc on prend son adresse
      Vec2* q = vect_sub(&(p+i)->next_pos, scalar_mult(scalar_product(vect_sub(&(p+i)->next_pos, &context->plan.point),&context->plan.normale),&context->plan.normale));
      Vec2* diff_C = vect_sub(&(p+i)->next_pos, q); //q est un pointeur
      float C = scalar_product(diff_C,&context->plan.normale) - (p+i)->radius;
      Vec2* deltai = scalar_mult(-2*C,&context->plan.normale);
      Vec2* final = vect_sum(&(p+i)->next_pos,deltai);
      (p+i)->next_pos = *final;
    }*/
    

    //choc avec le plan fait avec ma méthode et qui rebondit
    Vec2* difference = initialise_vec2();
    difference = vect_sub(&(p+i)->next_pos,&context->plan.point);
    if ( scalar_product(&context->plan.normale,difference)<=(p+i)->radius ) {
      Vec2* ecart_avantrebond=initialise_vec2();
      ecart_avantrebond = vect_sub(&(p+i)->position,&(p+i)->next_pos);
      Vec2* ecart_apresrebond = initialise_vec2();
      ecart_apresrebond->x = -ecart_avantrebond->x; //le symétrique
      ecart_apresrebond->y = ecart_avantrebond->y;
      Vec2* pos_apresrebond = vect_sum(&(p+i)->position,vect_sub(ecart_apresrebond,ecart_avantrebond));
      (p+i)->position=(p+i)->next_pos;
      (p+i)->next_pos=*pos_apresrebond;
    }

    //choc avec les boules bleues :

    for (int j=0; j<context->num_ground_sphere; j++) {

      //detection du choc
      float sdf = norme(vect_sub(&(p+i)->next_pos, &(s+j)->center)) - (s+j)->radius - (p+i)->radius;
      if (sdf<0) {
        Vec2* n_rebond = initialise_vec2();
        n_rebond = scalar_mult(1/norme(vect_sub(&(p+i)->next_pos,&(s+j)->center)),vect_sub(&(p+i)->next_pos,&(s+j)->center));
        Vec2* pcontact = initialise_vec2();
        pcontact = vect_sub(&(p+i)->next_pos,scalar_mult(sdf,n_rebond));

      //calcul du rebond que j'ai fait moi même car je n'arrive pas à implémenter les equations
      //je calcule un vecteur symétrique par rapport à la normale en changeant de base
      //je n'ai pas pris le temps d'implémenter une multiplication matricielle
      Vec2* ecart = vect_sub(&(p+i)->next_pos,pcontact);
      Vec2* Bprime_ecart = initialise_vec2();
      Bprime_ecart->x = n_rebond->x * ecart->x + n_rebond->y * ecart->y;
      Bprime_ecart->y = n_rebond->y * ecart->x - n_rebond->x * ecart->y;
      Vec2* Bprime_next_ecart = initialise_vec2();
      Bprime_next_ecart->x = - Bprime_ecart->x;
      Bprime_next_ecart->y = Bprime_ecart->y;
      Vec2* Bnext_ecart = initialise_vec2();
      Bnext_ecart->x = -1/(pow(n_rebond->x,2) + pow(n_rebond->y,2)) * (-n_rebond->x * Bprime_next_ecart->x + -n_rebond->y * Bprime_next_ecart->y);
      Bnext_ecart->y = -1/(pow(n_rebond->x,2) + pow(n_rebond->y,2)) * (-n_rebond->y * Bprime_next_ecart->x + n_rebond->x * Bprime_next_ecart->y);
      (p+i)->position = (p+i)->next_pos;
      (p+i)->next_pos=*vect_sum(scalar_mult(1,Bnext_ecart),pcontact);
      //Vec2* ecb = normalisation(ecart);
      //Vec2* affec = normalisation(Bprime_ecart);
      //Vec2* affnex = normalisation(Bprime_next_ecart);
      //printf("\n ecartbrut_av : %f %f \necart_av : %f %f\n pcontact : %f %f\n normale : %f %f \n ecart_ap :  %f %f\n",ecb->x,ecb->y,affec->x,affec->y,pcontact->x,pcontact->y,n_rebond->x, n_rebond->y, affnex->x,affnex->y);
      }
    }
  }
}

void projectConstraints(Context* context)
{
}

void updateVelocityAndPosition(Context* context, float dt)
{
  if (context->num_particles==0) return;
  Particle *p = context->particles;
  for (int i = 0; i<context->num_particles; i++){
    (p+i)->velocity.y = (1.0/dt) * ((p+i)->next_pos.y - (p+i)->position.y);
    (p+i)->position.y = (p+i)->next_pos.y;
    (p+i)->velocity.x = (1.0/dt) * ((p+i)->next_pos.x - (p+i)->position.x);
    (p+i)->position.x = (p+i)->next_pos.x;
  }
}

void applyFriction(Context* context)
{
  if (context->num_particles==0) return;
  Particle *p = context->particles;
  for (int i = 0; i<context->num_particles; i++){
    (p+i)->velocity=*scalar_mult(0.9999,&(p+i)->velocity);
  }
}

void deleteContactConstraints(Context* context)
{
}

// ------------------------------------------------

/*
int main()
{
  Context* context = initializeContext(10);
  addParticle(context, 100, 5, 10, 1, 0);
  printf("%f\n", getParticle(context, 0).position.y);
  applyExternalForce(context,1);
  updateExpectedPosition(context,1);
  updateVelocityAndPosition(context, 1);
  printf("%f\n", getParticle(context,0).position.y);
  return 0;
}
*/
