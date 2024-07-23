#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

const double COLOR_OPACITY = 0;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_type_t asset_get_type(asset_t *asset) { 
  return asset->type;
}

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  image_asset_t *asset = (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);
  SDL_Texture *txtr = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  asset->texture = txtr;
  asset->body = NULL;
  return (asset_t *)asset;
}

asset_t *asset_make_image_with_body(const char *filepath, body_t *body) {
  image_asset_t *asset = (image_asset_t *)asset_init(ASSET_IMAGE, 
                                                     sdl_get_bounding_box(body));
  SDL_Texture *txtr = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  asset->texture = txtr;
  asset->body = body;
  return (asset_t *)asset;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  text_asset_t *asset = (text_asset_t *)asset_init(ASSET_FONT, bounding_box);
  TTF_Font *font = asset_cache_obj_get_or_create(ASSET_FONT, filepath);
  asset->font = font;
  asset->text = text;
  asset->color = color;
  return (asset_t *)asset;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  assert(image_asset == NULL || image_asset->type == ASSET_IMAGE);
  assert(text_asset == NULL || text_asset->type == ASSET_FONT);
  button_asset_t *asset =
      (button_asset_t *)asset_init(ASSET_BUTTON, bounding_box);
  asset->image_asset = (image_asset_t *)image_asset;
  asset->text_asset = (text_asset_t *)text_asset;
  asset->handler = handler;
  return (asset_t *)asset;
}

body_t *asset_get_body(asset_t *image) {
  image_asset_t *asset = (image_asset_t *)image;
  return asset->body;
}

void asset_on_button_click(asset_t *button, state_t *state, double x,
                           double y) {
  button_asset_t *asset = (button_asset_t *)button;
  if (asset->is_rendered && x >= asset->base.bounding_box.x &&
      x <= asset->base.bounding_box.x + asset->base.bounding_box.w &&
      y >= asset->base.bounding_box.y &&
      y <= asset->base.bounding_box.y + asset->base.bounding_box.y) {
    asset->handler(state);
    asset->is_rendered = false;
  }
}

void asset_render(asset_t *asset) {
  switch (asset->type) {
  case ASSET_IMAGE: {
    image_asset_t *ast = (image_asset_t *)asset;
    SDL_Texture *txtr = ast->texture;
    if (ast->body) {
      asset->bounding_box = sdl_get_bounding_box(ast->body);
    }
    SDL_Rect bounding_box = asset->bounding_box;
    sdl_render_image(txtr,
                      (vector_t){.x = bounding_box.x, .y=bounding_box.y},
                      (vector_t){.x = bounding_box.w, .y=bounding_box.h});
    break;
  }
  case ASSET_FONT: {
    text_asset_t *ast = (text_asset_t *)asset;
    TTF_Font *font = ast->font;
    SDL_Color col = {.r = ast->color.r,
                     .g = ast->color.g,
                     .b = ast->color.b,
                     .a = COLOR_OPACITY};
    sdl_render_text(
        ast->text, font,
        (vector_t){.x = asset->bounding_box.x, .y = asset->bounding_box.y}, col,
        asset->bounding_box.w, asset->bounding_box.h);
    break;
  }
  case ASSET_BUTTON: {
    button_asset_t *ast = (button_asset_t *)asset;
    if (ast->image_asset) {
      asset_render((asset_t *)ast->image_asset);
    }
    if (ast->text_asset) {
      asset_render((asset_t *)ast->text_asset);
    }
    ast->is_rendered = true;
    break;
  }
  default: {
    assert(false && "Invalid entry type!\n");
  }
  }
}

void asset_destroy(asset_t *asset) {
  free(asset);
}
