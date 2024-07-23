#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "forces.h"
#include "list.h"
#include "scene.h"

const size_t GUESS_NUM_BODIES = 5;
const size_t GUESS_NUM_FORCES = 5;

struct scene {
  size_t num_bodies;
  size_t num_forces;
  list_t *bodies;
  list_t *force_creators;
};

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene);
  scene->num_bodies = 0;
  scene->num_forces = 0;
  scene->bodies = list_init(GUESS_NUM_BODIES, (free_func_t)body_free);
  scene->force_creators = list_init(GUESS_NUM_FORCES, (free_func_t)force_free);
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_creators);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(index >= 0 && index < scene->num_bodies);
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
  scene->num_bodies++;
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(index >= 0 && index < scene->num_bodies);
  body_remove(list_get(scene->bodies, index));
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, free));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  force_entry_t *entry = force_entry_init(forcer, aux, bodies);
  list_add(scene->force_creators, entry);
  scene->num_forces++;
}

/**
 * Return true if the force needs to be removed, false otherwise
 *
 * @param body the body to be removed
 * @param bodies a list of bodies
 * @return true if bodies contains body, false otherwise
 */
static bool remove_force(body_t *body_remove, list_t *bodies) {
  for (size_t k = 0; k < list_size(bodies); k++) {
    body_t *body = list_get(bodies, k);
    if (body_remove == body) {
      return true;
    }
  }
  return false;
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < scene->num_forces; i++) {
    force_entry_t *entry = list_get(scene->force_creators, i);
    void *aux = forces_get_force_aux(entry);
    forces_get_force_creator(entry)(aux);
  }

  for (ssize_t i = 0; i < scene->num_bodies; i++) {
    body_t *body = list_get(scene->bodies, i);
    if (body_is_removed(body)) {
      for (ssize_t j = 0; j < scene->num_forces; j++) {
        force_entry_t *entry = list_get(scene->force_creators, j);
        list_t *bodies = entry->bodies;
        body_aux_t *aux = entry->aux;
        list_t *aux_bodies = aux->bodies;
        // check if body to be removed is in the force's bodies or force's aux's
        // bodies
        if (remove_force(body, bodies) || remove_force(body, aux_bodies)) {
          force_free(list_remove(scene->force_creators, j));
          scene->num_forces--;
          j--;
        }
      }
      body_free(list_remove(scene->bodies, i));
      scene->num_bodies--;
      i--;
    } else {
      body_tick(body, dt);
    }
  }
}