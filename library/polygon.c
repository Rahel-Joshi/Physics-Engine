#include "polygon.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct polygon {
  list_t *points;
  vector_t center;
  double rotation;
  vector_t velocity;
  rgb_color_t *color;
  double total_rot;
} polygon_t;

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *polygon = malloc(sizeof(polygon_t));
  assert(polygon);
  polygon->points = points;
  polygon->rotation = rotation_speed;
  polygon->velocity = initial_velocity;
  polygon->color = color_init(red, green, blue);
  polygon->center = polygon_centroid(polygon);
  polygon->total_rot = 0;

  return polygon;
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->points; }

void polygon_move(polygon_t *polygon, double time_elapsed) {
  polygon_rotate(polygon, polygon->rotation, polygon->center);
  polygon_translate(polygon, vec_multiply(time_elapsed, polygon->velocity));
}

void polygon_set_velocity(polygon_t *polygon, vector_t vel) {
  polygon->velocity = vel;
}

void polygon_free(polygon_t *polygon) {
  list_free(polygon->points);
  color_free(polygon->color);
  free(polygon);
}

vector_t *polygon_get_velocity(polygon_t *polygon) {
  return &(polygon->velocity);
}

double polygon_area(polygon_t *polygon) {
  double area = 0;
  list_t *points = polygon->points;

  // computes area using cross products
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *v1 = list_get(points, i);
    vector_t *v2 = list_get(points, (i + 1) % list_size(points));
    area += vec_cross(*v1, *v2);
  }

  return area / 2.0;
}

vector_t polygon_centroid(polygon_t *polygon) {
  double sum_cx = 0;
  double sum_cy = 0;
  list_t *points = polygon_get_points(polygon);

  // computes centroid using cross products
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *v_current = list_get(points, i);
    vector_t *v_next = list_get(points, (i + 1) % list_size(points));

    double cross_product = v_current->x * v_next->y - v_next->x * v_current->y;
    sum_cx += (v_current->x + v_next->x) * cross_product;
    sum_cy += (v_current->y + v_next->y) * cross_product;
  }

  vector_t centroid = {sum_cx / (6 * polygon_area(polygon)),
                       sum_cy / (6 * polygon_area(polygon))};

  return centroid;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  list_t *points = polygon_get_points(polygon);

  // for each point, add the translation
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *v = list_get(points, i);
    *v = vec_add(*v, translation);
  }

  polygon->center = vec_add(polygon->center, translation);
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  list_t *points = polygon_get_points(polygon);

  // for each point, rotate it by angle about the arg point
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *v = list_get(points, i);
    vector_t translated_vertex = {v->x - point.x, v->y - point.y};
    vector_t rotated_vertex = vec_rotate(translated_vertex, angle);
    v->x = rotated_vertex.x + point.x;
    v->y = rotated_vertex.y + point.y;
  }

  polygon->total_rot += angle;
  while (polygon->total_rot >= 2 * M_PI) {
    polygon->total_rot -= 2 * (M_PI);
  }

  polygon->center = polygon_centroid(polygon);
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  polygon->color = color;
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  vector_t translation = vec_subtract(centroid, polygon->center);
  polygon_translate(polygon, translation);
}

vector_t polygon_get_center(polygon_t *polygon) { return polygon->center; }

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon_rotate(polygon, rot - polygon->total_rot, polygon->center);
  polygon->total_rot = rot;
}

double polygon_get_rotation(polygon_t *polygon) { return polygon->total_rot; }
