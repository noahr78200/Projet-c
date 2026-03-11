/*
 * projectile.h
 */
#ifndef DEF_PROJECTILE_H
#define DEF_PROJECTILE_H

#include <SDL/SDL.h>

typedef struct {
    float  x, y;
    int    w, h;
    float  vx;
    int    actif;
    int    owner;       /* 0 = P1, 1 = P2 */
    int    degats;
    int    est_special;
    Uint8  r, g, b;
    float  age;
} Projectile;

void Projectile_init     (Projectile * p, float x, float y, float vx,
                          int owner, int degats, int est_special,
                          Uint8 r, Uint8 g, Uint8 b);
void Projectile_update   (Projectile * p, float dt);
void Projectile_render   (Projectile * p, SDL_Surface * ecran);
int  Projectile_collision(const Projectile * p,
                          float px, float py, int pw, int ph);

#endif