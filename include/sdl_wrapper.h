#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
  MOUSE_BUTTON = 6,
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

typedef void (*hover_handler_t)(double x, double y, void *state);

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);

/**
 * A mouse handler.
 * When a mouse is pressed or released, the handler is passed its value.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*mouse_handler_t)(double x, double y, void *state);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

void sdl_on_mouse(mouse_handler_t handler);

void sdl_on_hover(hover_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Renders text to the screen.
 *
 * @param message the text string to render.
 * @param font the TTF_Font object used to render the text.
 * @param loc the vector for the text location
 * @param color the SDL_Color for text color
 * @param text_width the width per character
 * @param text_height the height per character
 */
void sdl_render_text(const char *message, TTF_Font *font, const vector_t loc,
                     SDL_Color color, const int8_t text_width,
                     const int8_t text_height);

/**
 * Renders an SDL_Texture to the screen. The texture is scaled to fit the entire
 * window.
 *
 * @param img the SDL_Texture to render.
 * @param loc the vector location of image
 * @param size the vector size of image
 */
void sdl_render_image(SDL_Texture *img, const vector_t loc,
                      const vector_t size);

/**
 * Loads an image and creates an SDL_Texture from it. This texture can be used
 * for rendering in the window.
 *
 * @param image_path the file path to the image.
 * @return a pointer to the loaded SDL_Texture
 */
SDL_Texture *sdl_get_texture(const char *image_path);

/**
 * Creates a TTF_Font object whic can be used for rendering text which we
 need
 * for. the counter.
 *
 * @param image_path the file path to the font file to be loaded.
 * @param font_size font size
 * @return a pointer to the loaded TTF_Font, or NULL if the font could not be
 * loaded.
 */

TTF_Font *sdl_get_font(const char *image_path, int8_t font_size);

/**
 * Creates a SDL_Rect object for that bounds the body_t body
 *
 * @param body pointer to body_t object
 * @return SDL_Rect bounding box
 */
SDL_Rect sdl_get_bounding_box(body_t *body);

/**
 * Plays the .wav audio file
 * 
 * @param file path
 */
void sdl_play_sound(const char *file);

#endif // #ifndef __SDL_WRAPPER_H__
