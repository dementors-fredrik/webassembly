#include <SDL/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define WIDTH 640
#define HEIGHT 480
#define CENTER_X (WIDTH / 2.0)
#define CENTER_Y (HEIGHT / 2.0)

#define STARS 10000

#define SPEED_X 5
#define SPEED_Y 5
#define SPEED_Z 5

typedef struct {
  float x, y, z;
  float xv, yv, zv;
} star;

void update_stars(star *field, unsigned int size);

star *init_starfield(unsigned int stars) {
  star *field = (star *)malloc(sizeof(star) * stars);
  for (int s = 0; s < stars; s++) {
    unsigned int iz = (rand() % 3000);
    float z = (iz / 1000.0) + 1.;
    field[s].x = (rand() % lround(WIDTH * z) * 2.) - (WIDTH * z);
    field[s].y = (rand() % lround(HEIGHT * z) * 2.) - (HEIGHT * z);
    field[s].z = z + 1.0;
    field[s].xv = 1.0 + ((rand() % (SPEED_X * 10)) / 10.0);
    field[s].yv = 0.0; 
    field[s].zv = 0.01;
  }
  update_stars(field, stars);
  return field;
}

void draw_stars(SDL_Surface *screen, star *field, unsigned int size) {
  for (unsigned int s = 0; s < size; s++) {
    float z = field[s].z;

    unsigned int x = (field[s].x / z) + CENTER_X;
    unsigned int y = (field[s].y / z) + CENTER_Y;
    if (x > 1 && x < WIDTH - 2 && y > 1 && y < HEIGHT - 2) {
      unsigned char col = 0xff - ((0xff / 3) * (z - 1.0));
      if (col > 10) {
        if (*((Uint32 *)screen->pixels + y * WIDTH + x) < col) {
          *((Uint32 *)screen->pixels + (y - 1) * WIDTH + x) =
              SDL_MapRGBA(screen->format, col, col, col, 0xff);
          *((Uint32 *)screen->pixels + y * WIDTH + (x + 1)) =
              SDL_MapRGBA(screen->format, col, col, col, 0xff);
          *((Uint32 *)screen->pixels + y * WIDTH + (x - 1)) =
              SDL_MapRGBA(screen->format, col, col, col, 0xff);
          *((Uint32 *)screen->pixels + y * WIDTH + x) =
              SDL_MapRGBA(screen->format, col, col, col, 0xff);
        }
      }
    }
  }
}

void update_stars(star *field, unsigned int size) {
  for (unsigned int s = 0; s < size; s++) {
    field[s].z -= field[s].zv;
    if (field[s].z < 1.0) {
      field[s].z = 4.;
    }
    field[s].x -= field[s].xv;
    if (field[s].x < -2000.0) {
      field[s].x = 2000.0; // WIDTH*z;
    }
    field[s].y -= field[s].yv;
    if (field[s].y < -2000.0) {
      field[s].y = 2000.0;
    }
  }
}

extern "C" int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE);
  star *field = init_starfield(STARS);

  while (true) {
    if (SDL_MUSTLOCK(screen))
      SDL_LockSurface(screen);
    memset(screen->pixels, 0, WIDTH * HEIGHT * 4);

    update_stars(field, STARS);
    draw_stars(screen, field, STARS);
    if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);
    SDL_Flip(screen);
    emscripten_sleep(16);
  }

  SDL_Quit();

  return 0;
}
