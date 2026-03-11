/*
 * projectile.c
 */
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "projectile.h"
#include "game.h"

#define PROJ_DUREE_MAX 3.0f

void Projectile_init(Projectile * p, float x, float y, float vx,
                     int owner, int degats, int est_special,
                     Uint8 r, Uint8 g, Uint8 b) {
    p->x           = x;
    p->y           = y;
    p->vx          = vx;
    p->w           = est_special ? 22 : 12;
    p->h           = est_special ? 14 :  8;
    p->actif       = 1;
    p->owner       = owner;
    p->degats      = degats;
    p->est_special = est_special;
    p->r = r; p->g = g; p->b = b;
    p->age         = 0.f;
}

void Projectile_update(Projectile * p, float dt) {
    if (!p->actif) return;
    p->x   += p->vx * dt;
    p->age += dt;
    if (p->age > PROJ_DUREE_MAX ||
        p->x < -50.f || p->x > (float)SCREEN_W + 50.f) {
        p->actif = 0;
    }
}

void Projectile_render(Projectile * p, SDL_Surface * ecran) {
    if (!p->actif) return;

    int x  = (int)p->x;
    int y  = (int)p->y;
    int cx = x + p->w / 2;
    int cy = y + p->h / 2;

    if (p->est_special) {
        filledEllipseRGBA(ecran, cx, cy,
                          p->w / 2 + 5, p->h / 2 + 5,
                          p->r, p->g, p->b, 60);
        filledEllipseRGBA(ecran, cx, cy,
                          p->w / 2, p->h / 2,
                          p->r, p->g, p->b, 255);
        ellipseRGBA(ecran, cx, cy,
                    p->w / 2, p->h / 2,
                    255, 255, 255, 200);
    } else {
        boxRGBA(ecran, x, y, x + p->w, y + p->h,
                p->r, p->g, p->b, 255);
        rectangleRGBA(ecran, x, y, x + p->w, y + p->h,
                      255, 255, 255, 180);
    }
}

/* AABB */
int Projectile_collision(const Projectile * p,
                         float px, float py, int pw, int ph) {
    if (!p->actif) return 0;
    return (p->x < px + (float)pw) && (p->x + (float)p->w > px) &&
           (p->y < py + (float)ph) && (p->y + (float)p->h > py);
}