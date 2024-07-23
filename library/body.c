#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"

const double SIXTH = 0.1666667;

typedef struct body {
  polygon_t *poly;
  double mass;
  double prev_dt;
  vector_t prev_vel;
  vector_t force;
  vector_t impulse;
  bool removed;
  void *info;
  free_func_t info_freer;
} body_t;

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  assert(mass > 0);
  body_t *body = malloc(sizeof(body_t));
  assert(body);
  body->poly = polygon_init(shape, VEC_ZERO, 0.0, color.r, color.g, color.b);
  body->mass = mass;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->removed = false;
  body->info = info;
  body->info_freer = info_freer;
  body->prev_vel = VEC_ZERO; 
  body->prev_dt = 0;

  return body;
}

polygon_t *body_get_polygon(body_t *body) { 
  return body->poly; 
}

void *body_get_info(body_t *body) { 
  return body->info; 
}

void body_free(body_t *body) {
  polygon_free(body->poly);
  if (body->info_freer) {
    body->info_freer(body->info);
  }
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *poly_points = polygon_get_points(body->poly);
  list_t *new_points = list_init(list_size(poly_points), free);
  for (size_t i = 0; i < list_size(poly_points); i++) {
    vector_t *old_vec = list_get(poly_points, i);
    vector_t *new_vec = malloc(sizeof(vector_t));
    assert(new_vec);
    new_vec->x = old_vec->x;
    new_vec->y = old_vec->y;
    list_add(new_points, new_vec);
  }
  return new_points;
}

vector_t body_get_centroid(body_t *body) {
  return polygon_get_center(body->poly);
}

vector_t body_get_velocity(body_t *body) {
  return (vector_t){.x = polygon_get_velocity(body->poly)->x,
                    .y = polygon_get_velocity(body->poly)->y};
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_set_center(body->poly, x);
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

double body_get_rotation(body_t *body) {
  return polygon_get_rotation(body->poly);
}

void body_set_rotation(body_t *body, double angle) {
  polygon_set_rotation(body->poly, angle);
}

void body_tick(body_t *body, double dt) {
  vector_t force_contribution = vec_multiply(dt / body->mass, body->force);
  vector_t impulse_contribution = vec_multiply(1 / body->mass, body->impulse);
  vector_t total_velocity_change =
      vec_add(force_contribution, impulse_contribution);
  vector_t curr_vel = body_get_velocity(body);
  vector_t new_velocity =
      vec_add(curr_vel, total_velocity_change);
  
  vector_t pre_mean_simpson_vel = vec_add(vec_add(body->prev_vel, 
      vec_multiply(4, curr_vel)), new_velocity); // using
    // Simpson's rule to approximate the integral from curr_time - prev_dt
    // to curr_time + dt
  
  vector_t avg_simpson_vel = vec_multiply(SIXTH, pre_mean_simpson_vel);
    // note: Simpson will give the integral from curr_time - prev_dt to
    // curr_time + dt but we only want curr_time to curr_time + dt, so uses
    // delta_x = (dt + prev_dt / 6) * (dt / dt + prev_dt) = dt / 6. Since
    // polygon_move will handle moving at a velocity over a time of dt, we
    // divide by dt and use polygon_move to handle the multiplication by dt.
    // this means that we only need to multiply by 1/6.  
    // Since Simpson's rule gives a greater convergence than trapezoid method,
    // this is an improved integration scheme.

  polygon_set_velocity(body->poly, avg_simpson_vel);
  polygon_move(body->poly, dt);
  polygon_set_velocity(body->poly, new_velocity);

  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->prev_dt = dt;
  body->prev_vel = curr_vel;
}

double body_get_mass(body_t *body) {
  return body->mass; 
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_remove(body_t *body) {
  body->removed = true; 
}

bool body_is_removed(body_t *body) { 
  return body->removed; 
}
