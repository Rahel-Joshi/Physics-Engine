#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 30;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

void *asset_check_in_list(asset_type_t ty, const char *filepath);

static void asset_cache_free_entry(entry_t *entry) {
  switch (entry->type) {
  case ASSET_IMAGE: {
    SDL_DestroyTexture(entry->obj);
    break;
  }
  case ASSET_FONT: {
    TTF_CloseFont(entry->obj);
    break;
  }
  case ASSET_BUTTON: {
    asset_destroy(entry->obj);
    break;
  }
  default: {
    assert(false && "Invalid entry type!\n");
  }
  }
  free((void *)entry->filepath);
  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { 
  list_free(ASSET_CACHE);
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  void *pot_obj = asset_check_in_list(ty, filepath);
  if (pot_obj != NULL) {
    return pot_obj;
  }

  void *obj;
  switch (ty) {
  case ASSET_IMAGE: {
    obj = sdl_get_texture(filepath);
    break;
  }
  case ASSET_FONT: {
    obj = sdl_get_font(filepath, FONT_SIZE);
    break;
  }
  default: {
    assert(false && "Invalid entry type!\n");
  }
  }

  entry_t *new_ent = malloc(sizeof(entry_t));
  assert(new_ent);
  new_ent->filepath = filepath;
  new_ent->type = ty;
  new_ent->obj = obj;
  list_add(ASSET_CACHE, new_ent);

  return obj;
}

void *asset_check_in_list(asset_type_t ty, const char *filepath) {
  size_t len_list = list_size(ASSET_CACHE);
  for (size_t i = 0; i < len_list; i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->filepath != NULL && strcmp(entry->filepath, filepath) == 0) {
      assert(entry->type == ty);
      return entry->obj;
    }
  }
  return NULL;
}

void asset_cache_register_button(asset_t *button) {
  assert(button != NULL);
  assert(asset_get_type(button) == ASSET_BUTTON);
  entry_t *new_button = malloc(sizeof(entry_t));
  assert(new_button);
  new_button->filepath = NULL;
  new_button->type = ASSET_BUTTON;
  new_button->obj = button;
  list_add(ASSET_CACHE, new_button);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {
  size_t cache_size = list_size(ASSET_CACHE);
  for (size_t i = 0; i < cache_size; i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->type == ASSET_BUTTON) {
      asset_on_button_click(entry->obj, state, x, y);
    }
  }
}
