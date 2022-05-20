#include <SDL/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int WIDTH = 320;
int HEIGHT = 240;

int CENTER_X = (WIDTH / 2);
int CENTER_Y = (HEIGHT / 2);

void setSize(float w, float h) {
  WIDTH = w;
  HEIGHT = h;

  CENTER_X = (WIDTH / 2.0);
  CENTER_Y = (HEIGHT / 2.0);
}

#define INTERNAL_SPEED 20

#define STARS 3000

int SUBF = 4;

typedef struct {
  float x, y, z;
  float xv, yv, zv;
  float size;
} star;

float compare_with_tresh(float a, float b, float tresh) {
  if (a > b - tresh && a < b + tresh) {
    return true;
  }
  return false;
}

void update_stars(star *field, unsigned int size);

#define RANGE 20
#define RANGE_SCALE 1000.0

float *zbuffer;
Uint8 *cbuffer;

float *init_zbuffer() {
  return (float *)malloc(WIDTH * HEIGHT * sizeof(float));
}

Uint8 *init_cbuffer() {
  return (Uint8 *)malloc(WIDTH * HEIGHT * sizeof(Uint8));
}

void clear_zbuffer() {
  for (int j = 0; j < WIDTH * HEIGHT; j++) {
    zbuffer[j] = 20.0;
  }
}

void clear_cbuffer() {
  for (int j = 0; j < WIDTH * HEIGHT; j++) {
    cbuffer[j] = 0;
  }
}

void interpolate_z_from_cbuffer() {
  for (int j = 1; j < (WIDTH * HEIGHT) - 1; j++) {
    float delta = cbuffer[j - 1] - cbuffer[j];
    if (cbuffer[j] > 0) {
      zbuffer[j] += delta > 0 ? 0.1 : -0.1;
    } else {
      zbuffer[j] = 20.0;
    }
  }
}

float get_rand_range(float start, float end) {
  int istart = start * RANGE_SCALE;
  int iend = end * RANGE_SCALE;
  return ((rand() % (int)(iend - istart)) + istart) / RANGE_SCALE;
}

star *init_starfield(unsigned int stars) {
  star *field = (star *)malloc(sizeof(star) * stars);
  for (int s = 0; s < stars; s++) {
    field[s].x = get_rand_range(-RANGE / 2, RANGE / 2);
    field[s].y = get_rand_range(-RANGE / 2, RANGE / 2);
    field[s].z = get_rand_range(-RANGE / 2, RANGE / 2);
    //      field[s].y = sin(s/1000.0)*cos(s/20.0) * 10.0;
    //      field[s].z = cos(s/1000.0)*sin(s/20.0) * 10.0;
    field[s].xv =
        get_rand_range(0.004, 0.01); //(((rand() % 100) / 1000.0) / 5.0) + 0.01;
    field[s].yv = 0;                 // get_rand_range(-0.005, 0.005);
    field[s].zv = 0;                 // get_rand_range(-0.005, 0.005);
    field[s].size = get_rand_range(1, 5);
    ;
    if (rand() % 100 < 5) {
      field[s].size = get_rand_range(15, 100);
    }
  }
  update_stars(field, stars);
  return field;
}

float zA = 1.3, xA = 1.5, yA = 1.2, FOV = 25.0;
float targetZAng = 1.3, targetXAng = 1.5, targetYAng = 1.2, targetFOV = 25.0;
int nextSeed = 100 * INTERNAL_SPEED;

star rotate_star(star *the_star) {
  star projected;
  float x1 = the_star->x * cos(zA) - the_star->y * sin(zA);
  float y1 = the_star->y * cos(zA) + the_star->x * sin(zA);
  float z1 = the_star->z;

  float x2 = x1 * cos(xA) - z1 * sin(xA);
  float y2 = y1;
  float z2 = z1 * cos(xA) + x1 * sin(xA);

  float x3 = x2;
  float y3 = y2 * cos(yA) - z2 * sin(yA);
  float z3 = z2 * cos(yA) + y2 * sin(yA);

  projected.x = x3;
  projected.y = y3;
  projected.z = z3;
  projected.size = the_star->size;
  return projected;
}

#define PI 3.14159256

void rotate_camera() {
  if (nextSeed <= 0) {
    unsigned char rotateBits = rand() % 16;
    if (rotateBits & 0x1) {
      targetZAng = get_rand_range(-180 * (PI / 180.0), 360 * (PI / 180.0));
    }
    if (rotateBits & 0x2) {
      targetXAng = get_rand_range(-180 * (PI / 180.0), 360 * (PI / 180.0));
    }
    if (rotateBits & 0x4) {
      targetYAng = get_rand_range(-180 * (PI / 180.0), 360 * (PI / 180.0));
    }
    // if (rotateBits & 0x8 && compare_with_tresh(FOV, targetFOV, 1.0)) {
    if (targetFOV > 300.0) {
      targetFOV = get_rand_range(25.0, 50.0);
    } else {
      targetFOV = get_rand_range(150.0, 1000.0);
    }
    //}
    nextSeed = (400 * INTERNAL_SPEED) + (rand() % 1200) * INTERNAL_SPEED;
  }
  nextSeed--;

  FOV += (targetFOV - FOV) / ((float)INTERNAL_SPEED * 100.);

  if (xA < targetXAng) {
    xA += 0.005 / (float)INTERNAL_SPEED;
  }

  if (xA > targetXAng) {
    xA -= 0.005 / (float)INTERNAL_SPEED;
  }

  if (yA < targetYAng) {
    yA += 0.005 / (float)INTERNAL_SPEED;
  }

  if (yA > targetYAng) {
    yA -= 0.005 / (float)INTERNAL_SPEED;
  }

  if (zA < targetZAng) {
    zA += 0.005 / (float)INTERNAL_SPEED;
  }

  if (zA > targetZAng) {
    zA -= 0.005 / (float)INTERNAL_SPEED;
  }
}

//#define FOV (300.0)

inline void draw_pixel(SDL_Surface *screen, float x, float y, float z,
                       Uint32 color) {
  if (x > 0 && x < WIDTH && y > 0 && y < HEIGHT) {
    if (zbuffer[(int)y * WIDTH + (int)x] > z) {
      zbuffer[(int)(y * WIDTH) + (int)x] = z;
      *((Uint32 *)screen->pixels + (int)(y * WIDTH) + (int)x) = color;
    }
  }
}

void draw_filled_circle(SDL_Surface *screen, float x, float y, float z, int r) {
  float zBase = z * 15.0;
  for (int i = 0; i < r * 2; i++)
    for (int j = 0; j < r * 2; j++) {
      float d = (float)sqrt((i - r) * (i - r) + (j - r) * (j - r));
      if (d < r) {
        Uint32 color = SDL_MapRGBA(screen->format, (0x99 - (d)) - zBase,
                                   (0x99 - (d / 2)) - zBase,
                                   (0xBB - (d * 3)) - zBase, 0xff);
        draw_pixel(screen, x + i - (r / 2), y + j - (r / 2), z - (d * 2),
                   color);
      }
    }
}

void draw_shaded_circle(SDL_Surface *screen, float x, float y, float z, float r,
                        Uint32 color) {
  for (int i = 0; i < r * 2; i++)
    for (int j = 0; j < r * 2; j++) {
      float d = (float)sqrt((i - r) * (i - r) + (j - r) * (j - r));
      if (d < r)
        draw_pixel(screen, x + i - (r / 2), y + j - (r / 2), z,
                   color - (d * 2));
    }
}

float wave = 0.0;

int compare(const void *a, const void *b) {
  star as = *((star *)a);
  star bs = *((star *)b);

  if (compare_with_tresh(as.z, bs.z, 0.0001))
    return 0;
  else if (as.z < bs.z)
    return -1;
  else
    return 1;
}

void draw_stars(SDL_Surface *screen, star *field, unsigned int size) {

  rotate_camera();

  qsort(field, 6, sizeof(int), compare);

  for (unsigned int s = 0; s < size; s++) {
    star tmp = field[s];
    tmp = rotate_star(&tmp);
    float z = tmp.z;

    float Cz = 0.004;
    float fovScale = FOV / 1000.0;

    if (z > 0.5 * fovScale && z <= 5.0 * fovScale * 20.0) {
      unsigned int x = (tmp.x * FOV / (Cz + z)) + CENTER_X;
      unsigned int y = (tmp.y * FOV / (Cz + z)) + CENTER_Y;

      if (x > 0 && x < WIDTH && y > 0 && y < HEIGHT) {
        float size = fovScale * (tmp.size / (z * 0.8));
        if (size > 1. && size < 150.) {
          draw_filled_circle(screen, x, y, tmp.z, size);
        } else {
          float zBase = z * 10.0;
          Uint32 color =
              SDL_MapRGBA(screen->format, (0x77 - (zBase)),
                          (0x66 - (zBase / 2)), (0x99 - (zBase * 3)), 0xff);
          draw_pixel(screen, x, y, z, color);
        }
      }
    }
  }
}

float clamp_to_box(float value) {
  float HR = (float)RANGE / 2.0;

  if (value < -HR) {
    return HR;
  }
  if (value > HR) {
    return -HR;
  }
  return value;
}

void update_stars(star *field, unsigned int size) {
  for (unsigned int s = 0; s < size; s++) {
    field[s].z -= field[s].zv;
    field[s].z = clamp_to_box(field[s].z);

    field[s].x -= field[s].xv;
    field[s].x = clamp_to_box(field[s].x);

    field[s].y -= field[s].yv;
    field[s].y = clamp_to_box(field[s].y);
  }
}

inline Uint8 fade_value(Uint8 v, Uint8 speed) {
  const Uint8 fade_speed = speed;
  if (v - fade_speed > 0) {
    return v - fade_speed;
  } else if (v > 0) {
    return v - 1;
  }
  return v;
}

void blur_fade(SDL_Surface *screen) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      Uint32 *pixel = &((Uint32 *)screen->pixels)[y * WIDTH + x];
      Uint8 r, g, b, a;
      SDL_GetRGBA(*pixel, screen->format, &r, &g, &b, &a);

      r = fade_value(r, 32);
      g = fade_value(g, 32);
      b = fade_value(b, 18);
      a = fade_value(a, 16);

      *pixel = SDL_MapRGBA(screen->format, r, g, b, a);
    }
  }
}

EMSCRIPTEN_KEEPALIVE
extern "C" int run(int width, int height) {
  setSize(width, height);
  srand(time(NULL));

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen =
      SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
  star *field = init_starfield(STARS);
  zbuffer = init_zbuffer();
  cbuffer = init_cbuffer();
  clear_zbuffer();
  clear_cbuffer();

  while (true) {
    if (SDL_MUSTLOCK(screen))
      SDL_LockSurface(screen);
    clear_zbuffer();

    //  memset(screen->pixels, 0, WIDTH * HEIGHT * 4);
    blur_fade(screen);

    for (int speed = 0; speed < INTERNAL_SPEED; speed++) {
      update_stars(field, STARS);
      draw_stars(screen, field, STARS);
    }

    if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);
    SDL_Flip(screen);
    emscripten_sleep(5);
  }

  SDL_Quit();

  return 0;
}
