/*
 * player.c
 */
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <stdlib.h>
#include "player.h"
#include "game.h"

/* Tableau des personnages */
CharacterStats PERSONNAGES[NB_CHARS] = {
    /* nom        hp   atk  spe  vit    enRate  R    G    B    pR   pG   pB  */
    { "RYU-C",   100,  20,  40, 200.f, 15.f,  220,  60,  60, 255, 130, 130 },
    { "MALLOC",   85,  25,  50, 230.f, 18.f,   60, 160, 220, 130, 200, 255 },
    { "POINTER", 115,  18,  36, 180.f, 12.f,  220, 160,  60, 255, 200, 100 },
    { "STRUCT",   95,  22,  44, 210.f, 14.f,   60, 200, 100, 130, 255, 160 },
    { "ENUM",     80,  28,  56, 250.f, 22.f,  180,  60, 220, 220, 130, 255 },
    { "TYPEDEF", 110,  16,  32, 190.f, 11.f,  220, 200,  60, 255, 240, 130 },
    { "NULL_PTR",150,  99, 100, 270.f, 40.f,  255, 255, 255, 255, 255, 255 }
};

#define GRAVITY      600.f
#define JUMP_FORCE  -380.f
#define ENERGIE_MAX  100.f

void Player_init(Player * p, int id, int char_id) {
    CharacterStats * s = &PERSONNAGES[char_id];

    p->id          = id;
    p->w           = 40;
    p->h           = 80;
    p->x           = (id == 0) ? 150.f : (float)(SCREEN_W - 190);
    p->y           = (float)(GROUND_Y - 80);
    p->vx          = 0.f;
    p->vy          = 0.f;
    p->au_sol      = 1;
    p->face        = (id == 0) ? 1 : -1;

    p->hp_max      = s->hp_max;
    p->hp          = s->hp_max;
    p->atk         = s->atk;
    p->atk_special = s->atk_special;
    p->vitesse     = s->vitesse;
    p->energie_rate= s->energie_rate;
    p->energie     = 0.f;
    p->energie_max = ENERGIE_MAX;

    p->etat        = STATE_IDLE;
    p->etat_timer  = 0.f;
    p->cooldown_atk= 0.f;
    p->hit_flash   = 0;

    p->key_left = p->key_right = p->key_jump = p->key_defend = 0;

    p->est_ia   = 0;
    p->ia_timer = 0.f;

    p->r = s->r;  p->g = s->g;  p->b = s->b;
    p->pr= s->pr; p->pg= s->pg; p->pb= s->pb;
}

void Player_update(Player * p, float dt, Player * adversaire) {
    if (p->etat == STATE_DEAD) return;

    if (p->etat_timer   > 0.f) p->etat_timer   -= dt;
    if (p->cooldown_atk > 0.f) p->cooldown_atk -= dt;
    if (p->hit_flash    > 0)   p->hit_flash--;

    /* Recharge energie */
    p->energie += p->energie_rate * dt;
    if (p->energie > p->energie_max) p->energie = p->energie_max;

    /* IA */
    if (p->est_ia && adversaire)
        Player_update_ia(p, adversaire, dt);

    /* Blocage pendant attaque */
    if (p->etat == STATE_ATTACK || p->etat == STATE_SPECIAL) {
        if (p->etat_timer <= 0.f) p->etat = STATE_IDLE;
        else {
            if (!p->au_sol) { p->vy += GRAVITY * dt; p->y += p->vy * dt; }
            return;
        }
    }

    /* Invincibilite apres coup */
    if (p->etat == STATE_HURT) {
        if (p->etat_timer <= 0.f) p->etat = STATE_IDLE;
        else return;
    }

    /* Defense */
    if (p->key_defend && p->au_sol) {
        p->etat = STATE_DEFEND;
        p->vx   = 0.f;
        return;
    }
    if (p->etat == STATE_DEFEND) p->etat = STATE_IDLE;

    /* Saut */
    if (p->key_jump && p->au_sol) {
        p->vy     = JUMP_FORCE;
        p->au_sol = 0;
        p->etat   = STATE_JUMP;
    }

    /* Deplacement */
    if (p->key_left)       { p->vx = -p->vitesse; p->face = -1; }
    else if (p->key_right) { p->vx =  p->vitesse; p->face =  1; }
    else                   { p->vx = 0.f; }

    /* Gravite */
    if (!p->au_sol) p->vy += GRAVITY * dt;

    /* Integration */
    p->x += p->vx * dt;
    p->y += p->vy * dt;

    /* Sol */
    if (p->y + (float)p->h >= (float)GROUND_Y) {
        p->y      = (float)(GROUND_Y - p->h);
        p->vy     = 0.f;
        p->au_sol = 1;
        if (p->etat == STATE_JUMP) p->etat = STATE_IDLE;
    }

    /* Bords */
    if (p->x < 0.f)                            p->x = 0.f;
    if (p->x + (float)p->w > (float)SCREEN_W)  p->x = (float)(SCREEN_W - p->w);

    /* Etat au sol */
    if (p->au_sol && p->etat != STATE_ATTACK && p->etat != STATE_SPECIAL
        && p->etat != STATE_HURT)
        p->etat = (p->vx != 0.f) ? STATE_RUN : STATE_IDLE;

    /* Facing vers adversaire */
    if (adversaire)
        p->face = (adversaire->x > p->x) ? 1 : -1;
}

void Player_render(Player * p, SDL_Surface * ecran) {
    if (p->etat == STATE_DEAD) return;
    if (p->hit_flash > 0 && p->hit_flash % 4 < 2) return;

    Uint8 r = p->r, g = p->g, b = p->b;
    if (p->etat == STATE_DEFEND) { r = 100; g = 100; b = 255; }
    if (p->etat == STATE_SPECIAL){ r = 255; g = 255; b = 255; }

    int x  = (int)p->x;
    int y  = (int)p->y;
    int w  = p->w;
    int h  = p->h;
    int cx = x + w / 2;

    /* Ombre */
    if (p->au_sol)
        filledEllipseRGBA(ecran, cx, GROUND_Y + 5, w / 2, 5, 0, 0, 0, 80);

    /* Jambes avec animation */
    int jy = y + 2 * h / 3;
    int jh = h / 3;
    int decal = (p->etat == STATE_RUN) ? ((SDL_GetTicks() / 80) % 2 ? 6 : -6) : 0;
    boxRGBA(ecran, x + 2,       jy + decal, x + w/2 - 2, jy + jh, r, g, b, 255);
    boxRGBA(ecran, x + w/2 + 2, jy - decal, x + w - 2,   jy + jh, r, g, b, 255);

    /* Corps */
    boxRGBA(ecran, x, y + h/5, x + w, y + 2*h/3, r, g, b, 255);

    /* Bras */
    int by = y + h/5 + 4;
    int bh = h / 4;
    if (p->etat == STATE_ATTACK || p->etat == STATE_SPECIAL) {
        int bx1 = (p->face == 1) ? x + w      : x - 25;
        int bx2 = (p->face == 1) ? x + w + 25 : x;
        boxRGBA(ecran, bx1, by, bx2, by + bh/2, r, g, b, 255);
    } else {
        boxRGBA(ecran, x - 8,    by, x,         by + bh, r, g, b, 200);
        boxRGBA(ecran, x + w,    by, x + w + 8, by + bh, r, g, b, 200);
    }

    /* Tete */
    filledCircleRGBA(ecran, cx, y + h/8, w/3, r, g, b, 255);

    /* Yeux */
    int oe = (p->face == 1) ? cx + 4 : cx - 6;
    filledCircleRGBA(ecran, oe, y + h/8 - 2, 3, 255, 255, 255, 255);
    filledCircleRGBA(ecran, oe + (p->face == 1 ? 1 : -1), y + h/8 - 2, 2, 20, 20, 20, 255);

    /* Bouclier defense */
    if (p->etat == STATE_DEFEND) {
        int bsx = (p->face == 1) ? x + w : x - 18;
        boxRGBA      (ecran, bsx, y + 10, bsx + 16, y + h - 10, 100, 100, 255, 180);
        rectangleRGBA(ecran, bsx, y + 10, bsx + 16, y + h - 10, 180, 180, 255, 255);
    }
}

void Player_prendre_coup(Player * p, int degats, int est_special) {
    if (p->etat == STATE_DEAD) return;
    if (p->etat == STATE_DEFEND) {
        if (!est_special) return;
        degats = degats * 30 / 100;
    }
    p->hp -= degats;
    if (p->hp < 0) p->hp = 0;
    p->hit_flash  = 16;
    p->etat       = STATE_HURT;
    p->etat_timer = 0.2f;
    if (p->hp == 0) p->etat = STATE_DEAD;
}

int Player_est_vivant(const Player * p) {
    return p->hp > 0 && p->etat != STATE_DEAD;
}

void Player_update_ia(Player * p, Player * ennemi, float dt) {
    p->ia_timer -= dt;
    if (p->ia_timer > 0.f) return;
    p->ia_timer = 0.15f + (float)(rand() % 20) / 100.f;

    float dist     = ennemi->x - p->x;
    float abs_dist = (dist < 0.f) ? -dist : dist;

    p->key_left = p->key_right = p->key_jump = p->key_defend = 0;

    if (abs_dist > 200.f) {
        if (dist > 0.f) p->key_right = 1;
        else            p->key_left  = 1;
    }

    if (p->au_sol && rand() % 10 == 0)
        p->key_jump = 1;
}