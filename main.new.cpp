#include <SDL/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define WIDTH 800
#define HEIGHT 600
#define CENTER_X (WIDTH / 2.0)
#define CENTER_Y (HEIGHT / 2.0)

#define STARS 100000

typedef struct {
  float x, y, z;
  float xv, yv, zv;
} star;

void update_stars(star *field, unsigned int size);

#define RANGE 5
#define RANGE_SCALE 1000.0

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
    field[s].xv = (((rand() % 100) / 1000.0) / 5.0) + 0.01;
    field[s].yv = 0.0; //(((rand()%100) / 1000.0)/5.0) + 0.01;
    field[s].zv = 0.0; //(((rand()%100) / 1000.0)/5.0) + 0.01;
  }
  update_stars(field, stars);
  return field;
}

float zA = 0.0, xA = 0.0, yA = 0.0;

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
  return projected;
}

int nextSeed = 0;
float targetZAng, targetXAng, targetYAng;

#define PI 3.14159256

void rotate_camera() {
    if(nextSeed <= 0) {
        unsigned char rotateBits = rand() % 7;
        if(rotateBits & 0x1){
            targetZAng = get_rand_range(-180*(PI/180.0),180*(PI/180.0));
        }
        if(rotateBits & 0x2){
            targetXAng = get_rand_range(-180*(PI/180.0),180*(PI/180.0));
        }
        if(rotateBits & 0x4){
            targetYAng = get_rand_range(-180*(PI/180.0),180*(PI/180.0));
        }
        nextSeed = 400 + rand()%1200;
    }
    nextSeed--;
    if(xA < targetXAng) {
        xA +=0.005;
    }
    
    if(xA > targetXAng) {
        xA -=0.005;
    }
    
    if(yA < targetYAng) {
        yA +=0.005;
    }
    
    if(yA > targetYAng) {
        yA -=0.005;
    }
    
    if(zA < targetZAng) {
        zA +=0.005;
    }
    
    if(zA > targetZAng) {
        zA -=0.005;
    }
}

void draw_stars(SDL_Surface *screen, star *field, unsigned int size) {
  
    rotate_camera();
    
  for (unsigned int s = 0; s < size; s++) {
    star tmp = field[s];
    tmp = rotate_star(&tmp);

    float z = tmp.z;
    float Cz = 0.004;
    if (z > 1.0 && z <= 3.0) {
      unsigned int x = (tmp.x * 400.0 / (Cz + z)) + CENTER_X;
      unsigned int y = (tmp.y * 400.0 / (Cz + z)) + CENTER_Y;
      if (x > 1 && x < WIDTH - 2 && y > 1 && y < HEIGHT - 2) {
        unsigned char col = 0xff - (z * 50.0);
        if (col > 1) {
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

extern "C" int main(int argc, char **argv) {
  srand(time(NULL));

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
    emscripten_sleep(1);
  }

  SDL_Quit();

  return 0;
}
