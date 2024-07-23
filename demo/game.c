#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"


typedef enum { BALL, HOLE, POLE, OBSTACLE, BOUNCY, WALL, RAMP, ARROW} body_type_t; 
    // what type of body is it?
typedef enum { TR_BL, TL_BR} empty_corners_t; // if the top right and bottom
    // left are randomly left empty or if the top left and bottom right are.
    // see add_random_walls().

const size_t INIT_CAPACITY = 4;

const vector_t WIND_MIN = {0, 0};
const vector_t WIND_MAX = {1000, 500};

const char *GRASS_BACKGROUND_PATH = "assets/background.png";

const SDL_Rect WINDOW = (SDL_Rect){.x = 0, .y = 0, .w = 1000, .h = 500};

const size_t TOTAL_HOLES = 9;
const size_t NUM_POINTS_IN_CIRCLE = 20;
const double BALL_RADIUS = 15;
const double BALL_MASS = 5;
const rgb_color_t BALL_WHITE = {1, 1, 1};
const double BOUNCY_CIRCLE_RADIUS = 45;
const rgb_color_t BOUNCY_CIRCLE_ORANGE = {0.85, 0.75, 0.2};
const double POLE_WIDTH = 5;
const double POLE_HEIGHT = 50;
const double HOLE_RADIUS = 20;
const rgb_color_t HOLE_DARK = {0.2, 0.2, 0.2};
const rgb_color_t NOT_TURN_WHITE = {255, 255, 255};
const rgb_color_t TURN_RED = {200, 25, 25};
const char *BALL_PATH = "assets/ball.png";
const char *HOLE_PATH = "assets/hole.png";
const char *POLE_PATH = "assets/pole.png";
const char *WALL_PATH = "assets/walls.png";
const char *BOUNCY_CIRCLE_PATH = "assets/bouncy_circle.png";
const char *TRANSLATING_OBSTACLE_PATH = "assets/translating_obstacle.png";
const char *const FONT_PATH = "assets/Cascadia.ttf";
const char *BACKGROUND_SONG_PATH = "assets/background_sound.wav";

const double ELASTICITY = 1;
const double BOUNCY_CIRCLE_ELASTICITY = 2.5;

const rgb_color_t WALL_GRAY = {0.5, 0.5, 0.5};
const size_t OUTER_WALL_WIDTH = 5;
const size_t WALL_STEP_SIZE = 100;
const size_t MAX_WALL_WIDTH = 400; // no random walls will be in the x range
                          // [MAX_WALL_WIDTH, WIND_MAX.x - MAX_WALL_WIDTH]

const vector_t RESET_POS = {500, 45};
const size_t TEXT_WIDTH = 15;
const size_t TEXT_HEIGHT = 25;
const size_t TEXT_WIDTH2 = 30;
const size_t TEXT_HEIGHT2 = 100;
const char *P1_TEXT = "Player 1: ";
const char *P2_TEXT = "Player 2: ";
const char *HOLE_TEXT = "Hole: ";
const vector_t P1_LOC = {25, 25};
const vector_t P1_COUNT_LOC = {200, 25};
const vector_t P2_LOC = {775, 25};
const vector_t P2_COUNT_LOC = {950, 25};
const vector_t HOLE_LOC = {450, 25};
const vector_t HOLE_COUNT_LOC = {550, 25};
const vector_t POWER_LOC = {450, 450};
const rgb_color_t BLACK = {0, 0, 0};

const vector_t TITLE_LEFT_LOC = {20, 100};
const vector_t TITLE_RIGHT_LOC = {770, 100};
const char *const TITLE_TEXT = "HOPPER GOLF";
const vector_t SCREEN_LOC = {250, 0};
const vector_t SCREEN_SIZE = {500, 500};
const char *const PLAY_SCREEN_PATH = "assets/play_screen.png";
const char *const END_SCREEN_PATH = "assets/end_screen.png";
const double GOLF_SWING_SPEED_THRESHOLD = 5.0;
const char *const GOLF_SWING_AUDIO = "assets/golf_swing.wav";
const double FORCE_MULTIPLIER = 1e4;
const double MAX_POWER = 10.0;
const double MIN_POWER = 1.0;
const double MAX_SPEED = 1e3;
const char *const PLAY = "PLAY";
const char *const BUTTON_PATH = "assets/button.png";
const SDL_Rect BUTTON_IMAGE_BOX = {425, 400, 100, 100};
const SDL_Rect BUTTON_TEXT_BOX = {450, 425, 50, 50};
const char *const GOLF_HOLE_AUDIO = "assets/golf_hole.wav";
const double RAMP_HEIGHT = 30.0;
const char *const UP_RAMP_PATH = "assets/up_ramp.png";
const char *const DOWN_RAMP_PATH = "assets/down_ramp.png";
const char *P1_WON_TEXT = "Player 1 won!";
const char *P2_WON_TEXT = "Player 2 won!";
const char *TIE_TEXT = "Tie!";
const double RAMP_SLOPE = 20.0;
const char *POWER_TEXT = "Power: ";

const double TRANSLATING_OBSTACLE_WIDTH = 25;
const double TRANSLATING_OBSTACLE_HEIGHT = 105;
const vector_t TRANSLATING_OBSTACLE_VELOCITY = {0, 100};
const double TRANSLATING_OBSTACLE_SPEED = 100;
const double MIN_TRANSLATING_OBSTACLE_TRAVELED = 300;
const double TRANSLATING_OFFSET = 40;
const vector_t INVALID_TRANSLATING_POSITION = {-10, -10};

const double ROTATING_OBSTACLE_WIDTH = 5;
const double ROTATING_OBSTACLE_HEIGHT = 80;
const double ROTATION_SPEED =  M_PI / 50;
const double Y_ROTATING_OFFSET = 80;
const double ROTATING_OBSTACLE_UPPER_HALF = 333;
const double ROTATING_OBSTACLE_LOWER_HALF = 166;

const size_t BACKGROUND_VOLUME = 20;
const size_t BACKGROUND_CHANNEL = 1;

const vector_t ARROW_OFFSET = {50, 0};
const double ARROW_WIDTH = 20;
const double ARROW_HEIGHT = 5; 

const size_t MAX_GENERATING_ATTEMPTS = 1000;

// state fields
struct state {
  list_t *body_assets;
  list_t *text_assets;
  asset_t *background;
  scene_t *scene;
  size_t power;
  asset_t *ball;
  asset_t *hole;
  asset_t *pole;
  asset_t *circle;
  asset_t *translating_obstacle;
  body_t *rotating_obstacle;
  body_t *arrow;
  vector_t curr_ball_loc;
  list_t *play_screen;
  list_t *end_screen;
  list_t *button_assets;
  size_t p1_str_count;
  size_t p2_str_count;
  size_t p_turn;
  size_t hole_num;
  asset_t *up_ramp; 
  bool has_handlers;
  asset_t *down_ramp;
  bool arrow_point_direction;
  vector_t mouse_position;
  vector_t rotating_obstacle_position;
};

// screen fields
typedef struct screen {
  size_t num_captions;
  char **captions;
  vector_t *locations;
  const char *font_path;
  const char *bg_path;
  vector_t bg_loc;
  vector_t bg_size;
} screen_t;

// button fields
typedef struct button_info {
  const char *image_path;
  const char *font_path;
  SDL_Rect image_box;
  SDL_Rect text_box;
  rgb_color_t text_color;
  const char *text;
  button_handler_t handler;
} button_info_t;

// play and end screen templates
screen_t screen_templates[] = {
    {.num_captions = 2,
     .captions = (char *[]){(char *)TITLE_TEXT, (char *)TITLE_TEXT},
     .locations = (vector_t[]){TITLE_LEFT_LOC, TITLE_RIGHT_LOC},
     .font_path = FONT_PATH,
     .bg_path = PLAY_SCREEN_PATH,
     .bg_loc = SCREEN_LOC,
     .bg_size = SCREEN_SIZE},
    {.num_captions = 0,
     .captions = (char *[]){},
     .locations = (vector_t[]){},
     .font_path = FONT_PATH,
     .bg_path = END_SCREEN_PATH,
     .bg_loc = SCREEN_LOC,
     .bg_size = SCREEN_SIZE,
    }};

/**
 * Return body's type
 * 
 * @param body
 * @return body type
 */
body_type_t get_type(body_t *body) {
  return *(body_type_t *)body_get_info(body);
}

/**
 * Makes and returns body type
 * 
 * @param type of body
 * @return body type
 */
body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  assert(info);
  *info = type;
  return info;
}

/**
 * Return double from low to high
 * 
 * @param low
 * @param high
 * @return random double
 */
double rand_double(double low, double high) {
  return (high - low) * rand() / RAND_MAX + low;
}

/**
 * Makes list of circle vertices
 * 
 * @param center of circle
 * @param radius of circle
 * @return list of vertices
 */
list_t *make_circle(vector_t center, double radius) {
    list_t *c = list_init(NUM_POINTS_IN_CIRCLE, free);
    for (size_t i = 0; i < NUM_POINTS_IN_CIRCLE; i++) {
      double angle = 2 * M_PI * i / NUM_POINTS_IN_CIRCLE;
      vector_t *v = malloc(sizeof(*v));
      assert(v);
      *v = (vector_t){center.x + radius * cos(angle), center.y + radius * sin(angle)};
      list_add(c, v);
    }
    return c;
}

/**
 * Makes list of rectangle vertices
 * 
 * @param center of rectangle
 * @param width of rectangle
 * @param height of rectangle
 * @return list of vertices
 */

list_t *make_rectangle(vector_t center, double width, double height) {
    list_t *points = list_init(4, free);
    vector_t *p1 = malloc(sizeof(vector_t));
    assert(p1);
    *p1 = (vector_t){center.x - width / 2, center.y - height / 2};

    vector_t *p2 = malloc(sizeof(vector_t));
    assert(p2);
    *p2 = (vector_t){center.x + width / 2, center.y - height / 2};

    vector_t *p3 = malloc(sizeof(vector_t));
    assert(p3);
    *p3 = (vector_t){center.x + width / 2, center.y + height / 2};

    vector_t *p4 = malloc(sizeof(vector_t));
    assert(p4);
    *p4 = (vector_t){center.x - width / 2, center.y + height / 2};

    list_add(points, p1);
    list_add(points, p2);
    list_add(points, p3);
    list_add(points, p4);

    return points;
}

/**
 * Makes list of arrow vertices
 * 
 * @param center of arrow
 * @param width of arrow
 * @param height of arrow
 * @return list of vertices
 */

list_t *make_arrow(vector_t center, double width, double height) {
  list_t *points = list_init(7, free);

  vector_t *p1 = malloc(sizeof(vector_t));
  assert(p1);
  *p1 = (vector_t){center.x - width / 2, center.y + height / 2};

  vector_t *p2 = malloc(sizeof(vector_t));
  assert(p2);
  *p2 = (vector_t){center.x + width / 2, center.y + height / 2};

  vector_t *p3 = malloc(sizeof(vector_t));
  assert(p3);
  *p3 = (vector_t){center.x + width / 2, center.y + height / 2 + height / 3};

  vector_t *p4 = malloc(sizeof(vector_t));
  assert(p4);
  *p4 = (vector_t){center.x + width / 2 + width, center.y};

  vector_t *p5 = malloc(sizeof(vector_t));
  assert(p5);
  *p5 = (vector_t){center.x + width / 2, center.y - height / 2 - height / 3};

  vector_t *p6 = malloc(sizeof(vector_t));
  assert(p6);
  *p6 = (vector_t){center.x + width / 2, center.y - height / 2};

  vector_t *p7 = malloc(sizeof(vector_t));
  assert(p7);
  *p7 = (vector_t){center.x - width / 2, center.y - height / 2};

  list_add(points, p1);
  list_add(points, p2);
  list_add(points, p3);
  list_add(points, p4);
  list_add(points, p5);
  list_add(points, p6);
  list_add(points, p7);

  return points;
}

/**
 * Adds ball to scene and stores the ball asset to the state.
 * 
 * @param state
 * @param ball_position of ball
 */

void add_ball(state_t *state, vector_t ball_position) {
  list_t *ball_shape = make_circle(ball_position, BALL_RADIUS);
  body_t *ball = body_init_with_info(ball_shape, BALL_MASS, BALL_WHITE,
                          make_type_info(BALL), free);
  body_set_velocity(ball, VEC_ZERO);
  scene_add_body(state->scene, ball);

  asset_t *ball_asset = asset_make_image_with_body(BALL_PATH, ball);
  state->ball = ball_asset;
  list_add(state->body_assets, ball_asset);
}

/**
 * Adds hole to scene and stores the hole asset to the state.
 * 
 * @param state
 * @param hole_position of hole
 */
void add_hole(state_t *state, vector_t hole_position) {
  list_t *hole_shape = make_circle(hole_position, HOLE_RADIUS);
  body_t *hole = body_init_with_info(hole_shape, INFINITY, HOLE_DARK,
                make_type_info(HOLE), free);
  scene_add_body(state->scene, hole);

  asset_t *hole_asset = asset_make_image_with_body(HOLE_PATH, hole);
  state->hole = hole_asset;
  list_add(state->body_assets, hole_asset);
}

/**
 * Adds pole to scene and stores the pole asset to the state.
 * 
 * @param state
 * @param hole_position of hole where it will render on.
 */
void add_pole(state_t *state, vector_t hole_position) {
    list_t *pole_shape = make_rectangle((vector_t){hole_position.x, hole_position.y
                                        + (POLE_HEIGHT /2)}, POLE_WIDTH, POLE_HEIGHT);
    body_t *pole = body_init_with_info(pole_shape, INFINITY, BALL_WHITE, 
                                        make_type_info(POLE), free);
    scene_add_body(state->scene, pole);

    asset_t *pole_asset = asset_make_image_with_body(POLE_PATH, pole);
    state->pole = pole_asset;
    list_add(state->body_assets, pole_asset);
}

/**
 * Returns the random location of the yellow ball
 * 
 * @return location of ball
 */
vector_t get_random_bouncy_circle() {
  vector_t loc = (vector_t) {WIND_MAX.x/2,
        rand_double(BOUNCY_CIRCLE_RADIUS, WIND_MAX.y - BOUNCY_CIRCLE_RADIUS)};

  return loc;
}

/**
 * Assigns position of rotating obstacle to state.
 * 
 * @param state
 */
void get_random_rotating_obstacle_position(state_t *state) {
  vector_t position;
  vector_t bouncy_circle_loc = body_get_centroid(asset_get_body(state->circle));
  if (bouncy_circle_loc.y < WIND_MAX.y/2) {
    position = (vector_t){rand_double(bouncy_circle_loc.x, 
                          WIND_MAX.x - MAX_WALL_WIDTH - ROTATING_OBSTACLE_HEIGHT/2), 
                          rand_double(ROTATING_OBSTACLE_UPPER_HALF, WIND_MAX.y - WALL_STEP_SIZE)};
  } else{
    position = (vector_t){rand_double(bouncy_circle_loc.x, 
                          WIND_MAX.x - MAX_WALL_WIDTH - ROTATING_OBSTACLE_HEIGHT/2), 
                          rand_double(WIND_MIN.y + WALL_STEP_SIZE, ROTATING_OBSTACLE_LOWER_HALF)};
  }

  state->rotating_obstacle_position = position;
}

/**
 * Returns location of the translating obstacle
 * 
 * @param state
 * @return position of the translating obstacle
 */
vector_t get_random_translating_obstacle_position(state_t *state) {   
  vector_t bouncy_circle_loc = body_get_centroid(asset_get_body(state->circle));

  double x_pos = rand_double(MAX_WALL_WIDTH + TRANSLATING_OBSTACLE_WIDTH,
                   bouncy_circle_loc.x - TRANSLATING_OFFSET);
  double y_pos = WIND_MIN.y + TRANSLATING_OBSTACLE_HEIGHT;
  vector_t position = {x_pos, y_pos};
  return position;
}

/**
 * Adds the rotating obstacle to the scene
 * 
 * @param state
 * @return rotating obstacle body.
 */

body_t *add_rotating_obstacle(state_t *state) {
  vector_t position = state->rotating_obstacle_position;
  list_t *rotating_obstacle_shape = make_rectangle(position, ROTATING_OBSTACLE_WIDTH,
            ROTATING_OBSTACLE_HEIGHT);
  body_t *rotating_obstacle = body_init_with_info(rotating_obstacle_shape, INFINITY,
            BALL_WHITE, make_type_info(OBSTACLE), free);
  scene_add_body(state->scene, rotating_obstacle);
  return rotating_obstacle;
}

/**
 * Adds the mouse direction arrow to scene
 * 
 * @param state
 * @param arrow_points_left which is a bool representing whether arrow is left
 * @return arrow body.
 */

body_t *add_arrow(state_t *state, bool arrow_points_left) {
    vector_t ball_center = body_get_centroid(asset_get_body(state->ball));
    vector_t position;
    if (arrow_points_left) {
        position = vec_add(ball_center, vec_negate(ARROW_OFFSET));
    } else {
        position = vec_add(ball_center, ARROW_OFFSET);
    }

    list_t *arrow_shape = make_arrow(position, ARROW_WIDTH, ARROW_HEIGHT);
    body_t *arrow = body_init_with_info(arrow_shape, INFINITY, BLACK, 
              make_type_info(ARROW), free);
    if (arrow_points_left) {
      polygon_rotate(body_get_polygon(arrow), M_PI, position);
    }
    scene_add_body(state->scene, arrow);
    return arrow;
}

/**
 * Adds translating obstacle to scene
 * 
 * @param state
 */
void add_translating_obstacle(state_t *state) {
  vector_t position = get_random_translating_obstacle_position(state);
  list_t *translating_obstacle_shape = make_rectangle(position, 
          TRANSLATING_OBSTACLE_WIDTH, TRANSLATING_OBSTACLE_HEIGHT);
  body_t *translating_obstacle = body_init_with_info(translating_obstacle_shape, INFINITY,
          BALL_WHITE, make_type_info(OBSTACLE), free);
  body_set_velocity(translating_obstacle, TRANSLATING_OBSTACLE_VELOCITY);
  scene_add_body(state->scene, translating_obstacle);
  
  asset_t *translating_obstacle_asset = 
      asset_make_image_with_body(TRANSLATING_OBSTACLE_PATH, translating_obstacle);
  state->translating_obstacle = translating_obstacle_asset;
  list_add(state->body_assets, translating_obstacle_asset);
}

/**
 * Updates translating obstacle continuously.
 * 
 * @param translating_obstacle body of obstacle.
 */
void update_translating_object(body_t *translating_obstacle) {
  vector_t centroid = body_get_centroid(translating_obstacle);
  vector_t velocity = body_get_velocity(translating_obstacle);
  if ((velocity.y > 0 &&
      centroid.y + (TRANSLATING_OBSTACLE_HEIGHT / 2) >= WIND_MAX.y - WALL_STEP_SIZE) || 
      (velocity.y < 0 && 
      centroid.y - (TRANSLATING_OBSTACLE_HEIGHT / 2) <= WIND_MIN.y + WALL_STEP_SIZE)) {
    body_set_velocity(translating_obstacle, (vector_t){velocity.x, -velocity.y});
  }
}


/**
 * Return a list of assets for screen
 * 
 * @param i index of screen template
 * @return list of assets
 */
list_t *get_screen(size_t i) {
  screen_t curr = screen_templates[i];
  list_t *screen_assets = list_init(curr.num_captions,
                                  (free_func_t)asset_destroy);
  SDL_Rect bg_rect = {.x = curr.bg_loc.x,
                      .y = curr.bg_loc.y,
                      .w = curr.bg_size.x,
                      .h = curr.bg_size.y};
  asset_t *bg = asset_make_image(curr.bg_path, bg_rect);
  list_add(screen_assets, bg);

  for (size_t i = 0; i < curr.num_captions; i++) {
    SDL_Rect text_rect = {.x = curr.locations[i].x,
                          .y = curr.locations[i].y,
                          .w = TEXT_WIDTH2,
                          .h = TEXT_HEIGHT2};
    asset_t *text =
        asset_make_text(curr.font_path, text_rect, curr.captions[i], BLACK);
    list_add(screen_assets, text);
  }
  return screen_assets;
}

/**
 * Makes text asset and adds to state
 * 
 * @param loc of text asset
 * @param filepath of asset texture
 * @param text string
 * @param col of text color
 * @param state
 */
void make_text(vector_t loc, const char *filepath, char *text,
    rgb_color_t col, state_t *state) {
  SDL_Rect text_rect = {.x = loc.x, .y = loc.y,
      .w = TEXT_WIDTH, .h = TEXT_HEIGHT};
  asset_t *text_asset = asset_make_text(filepath, text_rect, text, col);
  list_add(state->text_assets, text_asset);
}

/**
 * 
 * Adds the required game texts to list of text assets in state
 * 
 * @param state
 */
void add_game_texts(state_t *state) {
  size_t length = list_size(state->text_assets);
  if (length != 0) {
    for (size_t i = 0; i < length; i++) {
      list_remove(state->text_assets, i); // makes sure that stroke count can be
          // updated by removing old versions
    }
  }
  char *p1_str = malloc(sizeof(char) * (state->p1_str_count % 10 + 2)); // sees how
      // many characters are in the stroke count
  assert(p1_str);
  sprintf(p1_str, "%zu", state->p1_str_count);
  char *p2_str = malloc(sizeof(char) * (state->p2_str_count % 10 + 2)); // sees how
      // many characters are in the stroke count
  assert(p2_str);
  sprintf(p2_str, "%zu", state->p2_str_count);
  char *hole_count = malloc(sizeof(char) * (state->p1_str_count % 10 + 2)); // sees
      // how many characters are in the hole count
  assert(hole_count);
  sprintf(hole_count, "%zu", state->hole_num);

  char *power_str = malloc(sizeof(size_t));
  assert(power_str);
  sprintf(power_str, "%s%zu", POWER_TEXT, state->power);

  if (state->p_turn == 1) {
    make_text(P1_LOC, FONT_PATH, (char *) P1_TEXT, TURN_RED, state);
    make_text(P1_COUNT_LOC, FONT_PATH, p1_str, TURN_RED, state);
    make_text(P2_LOC, FONT_PATH, (char *) P2_TEXT, NOT_TURN_WHITE, state);
    make_text(P2_COUNT_LOC, FONT_PATH, p2_str, NOT_TURN_WHITE, state);
  }
  else {
    make_text(P1_LOC, FONT_PATH, (char *) P1_TEXT, NOT_TURN_WHITE, state);
    make_text(P1_COUNT_LOC, FONT_PATH, p1_str, NOT_TURN_WHITE, state);
    make_text(P2_LOC, FONT_PATH, (char *) P2_TEXT, TURN_RED, state);
    make_text(P2_COUNT_LOC, FONT_PATH, p2_str, TURN_RED, state);
  }
  make_text(HOLE_LOC, FONT_PATH, (char *) HOLE_TEXT, NOT_TURN_WHITE, 
      state);
  make_text(HOLE_COUNT_LOC, FONT_PATH, hole_count, NOT_TURN_WHITE, state);
  make_text(POWER_LOC, FONT_PATH, power_str, NOT_TURN_WHITE, state);
}

/**
 * Mouse handler. Plays sound, applies force to ball
 * 
 * @param x position of mouse
 * @param y position of mouse
 * @param state
 */
void on_mouse(double x, double y, state_t *state) {
  body_t *ball = asset_get_body(state->ball);
  vector_t velocity = body_get_velocity(ball);
  if (vec_get_length(velocity) < GOLF_SWING_SPEED_THRESHOLD) {
    if (state->p_turn == 1) {
      state->p1_str_count++;
      add_game_texts(state);
    }
    else {
      state->p2_str_count++;
      add_game_texts(state);
    }
    sdl_play_sound(GOLF_SWING_AUDIO);
    vector_t center = body_get_centroid(ball);
    vector_t mouse = (vector_t){.x = x, .y = WIND_MAX.y - y};
    vector_t direction = vec_subtract(mouse, center);
    vector_t unit_direction = vec_multiply(1.0 / vec_get_length(direction), direction);
    size_t power = state->power;
    vector_t force = vec_multiply(FORCE_MULTIPLIER * power, unit_direction);
    body_add_force(ball, force);
  }
}

/**
 * Key handler. Changes power, updates game texts
 * 
 * @param key that was pressed
 * @param type of key event
 * @param held_time of key
 * @param state
 */
void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  if (type == KEY_PRESSED && type != KEY_RELEASED) {
    switch (key) {
    case UP_ARROW:
      state->power = fmin(MAX_POWER, state->power + 1);
      add_game_texts(state);
      break;
    case DOWN_ARROW:
      state->power = fmax(MIN_POWER, state->power - 1);
      add_game_texts(state);
      break;
    }
  }
}

/**
 * Sets rotationg of arrow based on where the mouse is.
 * 
 * @param state
 */

void on_hover(double x, double y,state_t *state) {
  state->mouse_position = (vector_t){.x = x, .y = 500 - y};
  body_t *ball = asset_get_body(state->ball);
  vector_t ball_center = body_get_centroid(ball);
  vector_t mouse_pos = state->mouse_position;
  vector_t direction = vec_subtract(mouse_pos, ball_center);
  double angle = atan2(direction.y, direction.x);
  if (state->arrow) {
    body_set_rotation(state->arrow, angle);
  }
}

/**
 * Button handler. Moves on to next hole and displays game texts
 * 
 * @param state
 */
void next_hole(state_t *state) {
  state->hole_num += 1;
  state->hole_num %= (TOTAL_HOLES + 1);
  add_game_texts(state);
}

// button template for play screen
button_info_t button_templates[] = {
  {.image_path = BUTTON_PATH,
    .font_path = FONT_PATH,
    .image_box = BUTTON_IMAGE_BOX,
    .text_box = BUTTON_TEXT_BOX,
    .text_color = NOT_TURN_WHITE,
    .text = PLAY,
    .handler = (void *)next_hole}};

/**
 * Takes care of setting and unsetting handlers
 * 
 * @param state
 */
void handle_handlers(state_t *state) {
  if (state->hole_num == 1 && !state->has_handlers) {
    sdl_on_key((key_handler_t)on_key);
    sdl_on_mouse((mouse_handler_t)on_mouse);
    state->has_handlers = true;
  }
  else if (state->hole_num == TOTAL_HOLES + 1 && state->has_handlers) {
    sdl_on_key(NULL);
    sdl_on_mouse(NULL);
    state->has_handlers = true;
  }
}

/**
 * Using `info`, registers button and returns button
 *
 * @param info the button info struct used to initialize the button
 * @return pointer to button asset
 */
asset_t *create_button_from_info(button_info_t info) {
  asset_t *image_asset = NULL;
  asset_t *text_asset = NULL;
  if (info.image_path != NULL) {
    SDL_Rect image_box = info.image_box;
    image_asset = asset_make_image(info.image_path, image_box);
  }

  if (info.font_path != NULL) {
    SDL_Rect text_box = info.text_box;
    text_asset =
        asset_make_text(info.font_path, text_box, info.text, info.text_color);
  }

  SDL_Rect combined_box;
  if (image_asset == NULL) {
    combined_box = info.text_box;
  } else {
    combined_box = info.image_box;
  }

  asset_t *button =
      asset_make_button(combined_box, image_asset, text_asset, info.handler);
  asset_cache_register_button(button);
  return button;
}

/**
 * Returns button
 * 
 * @param i index of button_template
 * @return pointer to button asset
 */
asset_t *get_button(size_t i) {
  button_info_t info = button_templates[i];
  asset_t *button = create_button_from_info(info);
  return button;
}

/**
 * Reduces the ball's speed if above MAX_SPEED
 * 
 */
void round_vel(body_t *body) {
  vector_t vel = body_get_velocity(body);
  double speed = vec_get_length(vel);
  if (speed > MAX_SPEED) {
    body_set_velocity(body, vec_multiply(MAX_SPEED / speed, vel));
  }
}

/**
 * Makes and adds wall
 * 
 * @param state
 * @param center of wall
 * @param width of wall
 * @param height of wall
 */
void make_wall(state_t *state, vector_t center, double width, double height) {
  list_t *wall_pts = make_rectangle(center, width, height);
  body_t *wall = body_init_with_info(wall_pts, INFINITY, WALL_GRAY,
      make_type_info(WALL), free);
  scene_add_body(state->scene, wall);

  asset_t *wall_asset = asset_make_image_with_body(WALL_PATH, wall);
  list_add(state->body_assets, wall_asset);
  create_physics_collision(state->scene, asset_get_body(state->ball), wall, ELASTICITY);
}

/**
 * Adds the outer walls
 * 
 * @param state
 */
void add_outer_walls(state_t *state) {
  make_wall(state, (vector_t) {.x = OUTER_WALL_WIDTH / 2,
      .y = WIND_MAX.y / 2}, OUTER_WALL_WIDTH, WIND_MAX.y);
  make_wall(state, (vector_t) {.x = WIND_MAX.x - 
      OUTER_WALL_WIDTH / 2, .y = WIND_MAX.y / 2}, OUTER_WALL_WIDTH, WIND_MAX.y);
  make_wall(state, (vector_t) {.x = WIND_MAX.x / 2, .y = 
      OUTER_WALL_WIDTH / 2}, WIND_MAX.x - 2 * OUTER_WALL_WIDTH, 
      OUTER_WALL_WIDTH);
  make_wall(state, (vector_t) {.x = WIND_MAX.x / 2, .y =
      WIND_MAX.y - OUTER_WALL_WIDTH / 2}, WIND_MAX.x - 2 * OUTER_WALL_WIDTH,
      OUTER_WALL_WIDTH);
}

/**
 * Adds random walls to game
 * 
 * @param staet
 * @return which corner to use
 */
empty_corners_t add_random_walls(state_t *state) {
  empty_corners_t intact_corners = (empty_corners_t) floor(rand_double(0, 2));
      // need to have either the top right and bot left empty or top left and
      // bot right empty for hole and ball. 0 represents top right/bot left; 
      // 1 represents top left/bot right.

  for (size_t y = WALL_STEP_SIZE / 2; y < WIND_MAX.y; y += WALL_STEP_SIZE) {
    for (size_t i = 0; i < 2; i ++) { // one set of walls per side of the screen
      if ((intact_corners == TR_BL && ((i == 0 && y == WALL_STEP_SIZE / 2) ||
          (i == 1 && y == WIND_MAX.y - WALL_STEP_SIZE / 2))) || 
          (intact_corners == TL_BR && ((i == 0 && y == WIND_MAX.y - 
          WALL_STEP_SIZE / 2) || (i == 1 && y == WALL_STEP_SIZE / 2)))) { // if
              // in the corners that are designated to be empty
            continue;
      }

      size_t num_steps = floor(rand_double(0, MAX_WALL_WIDTH / 
          WALL_STEP_SIZE + 1));
      double w = num_steps * WALL_STEP_SIZE;
      double center_x;
      if (i == 0) { // if making wall on left side of screen
        center_x = w / 2;
      }
      else { // if on right side of screen
        center_x = WIND_MAX.x - w / 2;
      }
      make_wall(state, (vector_t) {.x = center_x, .y = y}, w, WALL_STEP_SIZE);
    }
  }
  return intact_corners;
}

/**
 * Adds bouncy circle to game
 * 
 * @param state
 */
void add_bouncy_circle(state_t *state) {
  vector_t loc = get_random_bouncy_circle();

  list_t *circle_shape = make_circle(loc, BOUNCY_CIRCLE_RADIUS);
  body_t *circle = body_init_with_info(circle_shape, INFINITY, 
      BOUNCY_CIRCLE_ORANGE, make_type_info(BOUNCY), free);
  scene_add_body(state->scene, circle);

  asset_t *circle_asset = asset_make_image_with_body(BOUNCY_CIRCLE_PATH,
      circle);
  list_add(state->body_assets, circle_asset);
  state->circle = circle_asset;
}

/**
 * Places the ball and hole
 * 
 * @param state
 * @param empt which corner to use
 */
void place_ball_and_hole(state_t *state, empty_corners_t empt) {
  bool hole_at_top = (bool) floor(rand_double(0, 2));
  bool arrow_points_left = false;
  body_t *hole_body = asset_get_body(state->hole);
  body_t *ball_body = asset_get_body(state->ball);

  switch (empt) {
    case TR_BL: { // now, need to randomize if the ball is at the TR
        // or if the hole is at the TR 
      if (hole_at_top) {
        body_set_centroid(hole_body, (vector_t) {.x = WIND_MAX.x -
            WALL_STEP_SIZE / 2, .y = WIND_MAX.y - WALL_STEP_SIZE / 2});
        body_set_centroid(ball_body, (vector_t) {.x = WALL_STEP_SIZE / 2,
            .y = WALL_STEP_SIZE / 2});
        arrow_points_left = false;
      } 
      else {
        body_set_centroid(ball_body, (vector_t) {.x = WIND_MAX.x -
            WALL_STEP_SIZE / 2, .y = WIND_MAX.y - WALL_STEP_SIZE / 2});
        body_set_centroid(hole_body, (vector_t) {.x = WALL_STEP_SIZE / 2,
            .y = WALL_STEP_SIZE / 2});
        arrow_points_left = true;
      }
      break;
    }
    case TL_BR: {
      if (hole_at_top) {
        body_set_centroid(hole_body, (vector_t) {.x = WALL_STEP_SIZE / 2, 
            .y = WIND_MAX.y - WALL_STEP_SIZE / 2});
        body_set_centroid(ball_body, (vector_t) {.x = WIND_MAX.x - 
            WALL_STEP_SIZE / 2, .y = WALL_STEP_SIZE / 2});
        arrow_points_left = true;   
      } 
      else {
        body_set_centroid(ball_body, (vector_t) {.x = WALL_STEP_SIZE / 2, 
            .y = WIND_MAX.y - WALL_STEP_SIZE / 2});
        body_set_centroid(hole_body, (vector_t) {.x = WIND_MAX.x - 
            WALL_STEP_SIZE / 2, .y = WALL_STEP_SIZE / 2});
        arrow_points_left = false;    
      }
      break;
    }
  }
  state->curr_ball_loc = body_get_centroid(ball_body);
  body_t *pole = asset_get_body(state->pole);
  body_set_centroid(pole, body_get_centroid(hole_body));
  state->arrow_point_direction = arrow_points_left;
  state->arrow = add_arrow(state, arrow_points_left);
}

/**
 * 
 * Return vector_t of valid random ramp location
 * 
 * @param state
 * @param is_up_ramp true if up_ramp location, false if down_ramp location
 * 
 */
vector_t get_random_ramp_loc(state_t *state, bool is_up_ramp) {
  vector_t loc = VEC_ZERO;
      
  double circle_y = body_get_centroid(asset_get_body(state->circle)).y;
  double up_ramp_y = 0;
  if (!is_up_ramp) {
    up_ramp_y = body_get_centroid(asset_get_body(state->up_ramp)).y;
  }
  bool ramp_wall_proximity_bad = true;
  bool ramp_circle_proximity_bad = true;
  bool up_down_ramp_proximity_bad = true;

  while (ramp_wall_proximity_bad || ramp_circle_proximity_bad 
                            || up_down_ramp_proximity_bad) {
    loc = (vector_t) {WIND_MAX.x / 2,
           rand_double(WALL_STEP_SIZE, WIND_MAX.y - WALL_STEP_SIZE)};

    ramp_circle_proximity_bad = (loc.y >= circle_y - BOUNCY_CIRCLE_RADIUS - 
        RAMP_HEIGHT / 2 && loc.y <= circle_y + BOUNCY_CIRCLE_RADIUS + RAMP_HEIGHT / 2);

    ramp_wall_proximity_bad = (size_t)(loc.y) % WALL_STEP_SIZE <= RAMP_HEIGHT ||
            (size_t)(loc.y) % WALL_STEP_SIZE >= WALL_STEP_SIZE - RAMP_HEIGHT;

    up_down_ramp_proximity_bad = (!is_up_ramp) && (loc.y >= up_ramp_y - RAMP_HEIGHT &&
              loc.y <= up_ramp_y + RAMP_HEIGHT);
  }

  return loc;
}

/**
 * Makes the golf hole
 * 
 * @param state
 */
void make_hole(state_t *state) {
  add_outer_walls(state);
  vector_t loc = (vector_t) {WIND_MAX.x/2,
        rand_double(BOUNCY_CIRCLE_RADIUS, WIND_MAX.y - BOUNCY_CIRCLE_RADIUS)};;
  body_set_centroid(asset_get_body(state->circle), loc);
  body_set_centroid(asset_get_body(state->up_ramp), get_random_ramp_loc(state, true));
  body_set_centroid(asset_get_body(state->down_ramp), get_random_ramp_loc(state, false));
  get_random_rotating_obstacle_position(state);
  state->rotating_obstacle = add_rotating_obstacle(state);
  empty_corners_t empt = add_random_walls(state);
  place_ball_and_hole(state, empt);
}

/**
 * Collision handler for the golf ball and hole
 * 
 * @param body1
 * @param body2
 * @param axis 
 */
void end_hole(body_t *body1, body_t *body2, vector_t axis, void *aux, 
                                    double force_const) {
  state_t *state = aux;
  body_t *ball = asset_get_body(state->ball);
  body_set_velocity(ball, VEC_ZERO);
  body_reset(ball);
  if (state->p_turn == 1) {
    body_set_centroid(ball, state->curr_ball_loc);
    state->p_turn++;
  }
  else {
    for (ssize_t i = list_size(state->body_assets) - 1; i >= 0; i--) {
      body_t *body = asset_get_body(list_get(state->body_assets, i));
      if (get_type(body) == WALL) {
        body_remove(body);
        list_remove(state->body_assets, i);
      }
    }
    make_hole(state);
    state->hole_num++;
    state->p_turn = 1;
  }
   add_game_texts(state);
}

/**
 * Add forces between bodies
 * 
 * @param state
 */
void add_force_creators(state_t *state) {
  create_ramp_collision(state->scene, asset_get_body(state->ball), 
              asset_get_body(state->up_ramp), RAMP_SLOPE);
  create_ramp_collision(state->scene, asset_get_body(state->ball), 
              asset_get_body(state->down_ramp), -RAMP_SLOPE);
  create_drag(state->scene, 5.0, asset_get_body(state->ball));
  body_t *ball = asset_get_body(state->ball);
   for (size_t i = 0; i < scene_bodies(state->scene); i++) {
     body_t *body = scene_get_body(state->scene, i);
     switch (get_type(body)) {
     case BALL: 
       break; 
     case POLE: 
       break;
     case OBSTACLE: 
       create_physics_collision(state->scene, ball, body, ELASTICITY);
       break;
     case HOLE:
       create_collision(state->scene, ball, body, (collision_handler_t) end_hole, state,
       ELASTICITY);
       break;
     case BOUNCY:
        create_physics_collision(state->scene, body, asset_get_body(state->ball),
            BOUNCY_CIRCLE_ELASTICITY);
        break;
     case WALL:
        break;
     default:
       break;
     }
   }
}

/**
 * Check for collision between ball and hole
 * 
 * @param state
 */
void check_collisions(state_t *state) {
  body_t *ball = asset_get_body(state->ball);
  vector_t ball_center = body_get_centroid(ball);
  size_t bodies = scene_bodies(state->scene);

  for (size_t i = 0; i < bodies; i++) {
    body_t *body_i = scene_get_body(state->scene, i);
    if (get_type(body_i) != BALL) {
      if (get_type(body_i) == HOLE) {
        vector_t body_center = body_get_centroid(body_i);
        vector_t displacement = vec_subtract(ball_center, body_center);
        double distance = sqrt((displacement.x * displacement.x) +
                             (displacement.y * displacement.y));
        if (distance < BALL_RADIUS) {
          end_hole(ball, asset_get_body(state->hole), VEC_ZERO, state, ELASTICITY); 
        }
      }
    }
  }
}  

/**
 * 
 * Create the up or down ramps
 * 
 * @param state
 * @param is_up_ramp true if up_ramp location, false if down_ramp location
 * 
 */
void add_ramp(state_t *state, bool is_up_ramp) { 
  scene_t *scene = state->scene;
  list_t *body_assets = state->body_assets;
  list_t *shape = make_rectangle(get_random_ramp_loc(state, is_up_ramp), WIND_MAX.x, 
        RAMP_HEIGHT);
  body_t *ramp = body_init_with_info(shape, INFINITY, BOUNCY_CIRCLE_ORANGE, 
        make_type_info(RAMP), free);

  scene_add_body(scene, ramp);
  asset_t *ramp_asset = NULL; 
  if (is_up_ramp) {
    ramp_asset = asset_make_image_with_body(UP_RAMP_PATH, ramp);
  }
  else {
    ramp_asset = asset_make_image_with_body(DOWN_RAMP_PATH, ramp);
  }
  list_add(body_assets, ramp_asset);
  if (is_up_ramp) {
    state->up_ramp = ramp_asset;
  }
  else {
    state->down_ramp = ramp_asset;
  }
}

/**
 * Makes the end screen's text
 * 
 * @param state
 */
void make_end_screen_text(state_t *state) {
  char *text = NULL;
  if (state->p1_str_count < state->p2_str_count) {
    text = (char *)P1_WON_TEXT;
  }
  else if (state->p1_str_count > state->p2_str_count) {
    text = (char *)P2_WON_TEXT;
  }
  else {
    text = (char *)TIE_TEXT;
  }
  make_text(TITLE_LEFT_LOC, FONT_PATH, text, BLACK, state);
  make_text(TITLE_RIGHT_LOC, FONT_PATH, text, BLACK, state);
  asset_render(list_get(state->text_assets, list_size(state->text_assets)-2));
  asset_render(list_get(state->text_assets, list_size(state->text_assets)-1));
}

/** 
 * 
 * Render a list of assets
 * 
 * @param list of assets
 * @param len of list
 */
void display_assets(list_t *list, size_t len) {
  for (size_t i = 0; i < len; i++) {
    asset_render(list_get(list, i));
  }
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(WIND_MIN, WIND_MAX);
  TTF_Init();
  srand(time(NULL));

  Mix_Chunk *background_song = Mix_LoadWAV(BACKGROUND_SONG_PATH);
  Mix_PlayChannel(BACKGROUND_CHANNEL, background_song, -1);
  Mix_Volume(BACKGROUND_CHANNEL, BACKGROUND_VOLUME);

  state_t *state = malloc(sizeof(state_t));
  assert(state);
  state->power = MIN_POWER;
  
  state->has_handlers = false;
  state->play_screen = get_screen(0);
  list_add(state->play_screen, get_button(0));
  state->end_screen = get_screen(1);

  state->p1_str_count = 0;
  state->p2_str_count = 0;
  state->p_turn = 1;
  state->hole_num = 0;

  state->scene = scene_init();
  state->body_assets = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
  state->text_assets = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
  state->body_assets = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
  state->text_assets = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
  state->background = asset_make_image(GRASS_BACKGROUND_PATH, WINDOW);

  add_bouncy_circle(state);
  add_ramp(state, 1);
  add_ramp(state, 0);
  add_ball(state, RESET_POS); 
  add_hole(state, RESET_POS);
  add_pole(state, RESET_POS);
  make_hole(state);

  add_game_texts(state);

  get_random_rotating_obstacle_position(state);
  state->rotating_obstacle = add_rotating_obstacle(state);
  add_translating_obstacle(state);

  add_force_creators(state); 

  sdl_on_hover((hover_handler_t)on_hover);

  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  sdl_clear();
  handle_handlers(state);
  asset_render(state->background);
  if (state->hole_num == 0) {
    display_assets(state->play_screen, list_size(state->play_screen));
  }
  else if (state->hole_num == TOTAL_HOLES + 1) {
    display_assets(state->play_screen, list_size(state->end_screen));
    make_end_screen_text(state);
  }
  else {
    display_assets(state->body_assets, list_size(state->body_assets));
    display_assets(state->text_assets, list_size(state->text_assets));
    if (state->translating_obstacle) {
      body_t *translating_obstacle = asset_get_body(state->translating_obstacle);
      update_translating_object(translating_obstacle);
    }
    sdl_draw_polygon(body_get_polygon(state->rotating_obstacle), BALL_WHITE);
    polygon_rotate(body_get_polygon(state->rotating_obstacle), ROTATION_SPEED, 
            body_get_centroid(state->rotating_obstacle));

    if (vec_get_length(body_get_velocity(asset_get_body(state->ball))) <= 
          GOLF_SWING_SPEED_THRESHOLD) {
      if (state->arrow == NULL) {
        state->arrow = add_arrow(state, state->arrow_point_direction);
      }
      sdl_draw_polygon(body_get_polygon(state->arrow), BLACK);
    } else {
      if (state->arrow != NULL) {
        for (size_t i = 0; i < scene_bodies(state->scene); i++) {
          if (scene_get_body(state->scene, i) == state->arrow) {
            scene_remove_body(state->scene, i);
            break;
          }
        }
        state->arrow = NULL;
      }
    }
  }
  round_vel(asset_get_body(state->ball));
  check_collisions(state);
  sdl_show();
  scene_tick(state->scene, dt);
  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->body_assets);
  scene_free(state->scene); 
  asset_cache_destroy();
  free(state);
}