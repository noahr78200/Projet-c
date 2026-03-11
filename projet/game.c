/*
 * game.c ok ok
 */
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "menu.h"
#include "player.h"
#include "projectile.h"

/* Sequence easter egg */
static const SDLKey SECRET[8] = {
    SDLK_UP, SDLK_UP, SDLK_DOWN, SDLK_DOWN,
    SDLK_LEFT, SDLK_RIGHT, SDLK_LEFT, SDLK_RIGHT
};

/* -------------------------------------------------------
 * Cherche un slot libre dans le tableau de projectiles
 * (simulation malloc dans tableau statique - chap. 8)
 * ------------------------------------------------------- */
static Projectile * proj_alloc(Game * jeu) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!jeu->projectiles[i].actif)
            return &jeu->projectiles[i];
    }
    return NULL;
}

/* -------------------------------------------------------
 * Tirer un projectile depuis un joueur
 * ------------------------------------------------------- */
static void tirer(Game * jeu, Player * p, int est_special) {
    Projectile * proj = proj_alloc(jeu);
    if (proj == NULL) return;

    if (est_special && p->energie < ENERGIE_COST) return;

    float vx  = (float)p->face * (est_special ? 450.f : 350.f);
    float px  = (p->face == 1) ? p->x + (float)p->w : p->x - 22.f;
    float py  = p->y + (float)p->h * 0.35f;
    int degats= est_special ? p->atk_special : p->atk;

    Projectile_init(proj, px, py, vx, p->id, degats, est_special,
                    p->pr, p->pg, p->pb);

    p->etat         = est_special ? STATE_SPECIAL : STATE_ATTACK;
    p->etat_timer   = est_special ? 0.35f : 0.2f;
    p->cooldown_atk = est_special ? 0.9f  : 0.45f;

    if (est_special) p->energie -= ENERGIE_COST;
}

/* -------------------------------------------------------
 * Game_init
 * ------------------------------------------------------- */
void Game_init(Game * jeu, SDL_Surface * ecran) {
    int i;
    jeu->ecran_actif    = SCREEN_MENU;
    jeu->mode           = MODE_PVP;
    jeu->timer          = TIMER_COMBAT;
    jeu->timer_acc      = 0.f;
    jeu->p1_selection   = 0;
    jeu->p2_selection   = 1;
    jeu->select_phase   = 1;
    jeu->secret_len     = 0;
    jeu->secret_unlocked= 0;
    for (i = 0; i < MAX_PROJECTILES; i++) jeu->projectiles[i].actif = 0;
    for (i = 0; i < 8; i++) jeu->secret_buffer[i] = 0;
    (void)ecran;
}

/* -------------------------------------------------------
 * Game_start_fight
 * ------------------------------------------------------- */
void Game_start_fight(Game * jeu) {
    int i;
    Player_init(&jeu->p1, 0, jeu->p1_selection);
    Player_init(&jeu->p2, 1, jeu->p2_selection);
    if (jeu->mode == MODE_PVAI) jeu->p2.est_ia = 1;
    jeu->timer    = TIMER_COMBAT;
    jeu->timer_acc= 0.f;
    for (i = 0; i < MAX_PROJECTILES; i++) jeu->projectiles[i].actif = 0;
}

/* -------------------------------------------------------
 * Game_key_down
 * ------------------------------------------------------- */
void Game_key_down(Game * jeu, SDLKey sym) {
    int i;

    /* Easter egg : buffer circulaire */
    jeu->secret_buffer[jeu->secret_len % 8] = (int)sym;
    jeu->secret_len++;
    if (jeu->secret_len >= 8) {
        int ok = 1;
        for (i = 0; i < 8; i++) {
            if (jeu->secret_buffer[(jeu->secret_len - 8 + i) % 8]
                != (int)SECRET[i]) { ok = 0; break; }
        }
        if (ok) jeu->secret_unlocked = 1;
    }

    switch (jeu->ecran_actif) {

        case SCREEN_MENU:
            Menu_key_principal(jeu, sym);
            break;

        case SCREEN_SELECT:
            Menu_key_selection(jeu, sym);
            break;

        case SCREEN_FIGHT:
            /* P1 */
            if (sym == SDLK_a)     jeu->p1.key_left   = 1;
            if (sym == SDLK_d)     jeu->p1.key_right  = 1;
            if (sym == SDLK_w)     jeu->p1.key_jump   = 1;
            if (sym == SDLK_s)     jeu->p1.key_defend = 1;
            if (sym == SDLK_f && jeu->p1.cooldown_atk <= 0.f
                && jeu->p1.etat != STATE_DEAD)
                tirer(jeu, &jeu->p1, 0);
            if (sym == SDLK_g && jeu->p1.cooldown_atk <= 0.f
                && jeu->p1.etat != STATE_DEAD
                && jeu->p1.energie >= ENERGIE_COST)
                tirer(jeu, &jeu->p1, 1);

            /* P2 (humain uniquement) */
            if (!jeu->p2.est_ia) {
                if (sym == SDLK_LEFT)      jeu->p2.key_left   = 1;
                if (sym == SDLK_RIGHT)     jeu->p2.key_right  = 1;
                if (sym == SDLK_UP)        jeu->p2.key_jump   = 1;
                if (sym == SDLK_DOWN)      jeu->p2.key_defend = 1;
                if (sym == SDLK_l && jeu->p2.cooldown_atk <= 0.f
                    && jeu->p2.etat != STATE_DEAD)
                    tirer(jeu, &jeu->p2, 0);
                if (sym == SDLK_SEMICOLON && jeu->p2.cooldown_atk <= 0.f
                    && jeu->p2.etat != STATE_DEAD
                    && jeu->p2.energie >= ENERGIE_COST)
                    tirer(jeu, &jeu->p2, 1);
            }
            break;

        case SCREEN_RESULT:
            if (sym == SDLK_r) {
                Game_start_fight(jeu);
                jeu->ecran_actif = SCREEN_FIGHT;
            }
            if (sym == SDLK_ESCAPE) {
                jeu->ecran_actif  = SCREEN_MENU;
                jeu->p1_selection = 0;
            }
            break;
    }
}

/* -------------------------------------------------------
 * Game_key_up
 * ------------------------------------------------------- */
void Game_key_up(Game * jeu, SDLKey sym) {
    if (jeu->ecran_actif != SCREEN_FIGHT) return;

    if (sym == SDLK_a)     jeu->p1.key_left   = 0;
    if (sym == SDLK_d)     jeu->p1.key_right  = 0;
    if (sym == SDLK_w)     jeu->p1.key_jump   = 0;
    if (sym == SDLK_s)     jeu->p1.key_defend = 0;

    if (!jeu->p2.est_ia) {
        if (sym == SDLK_LEFT)  jeu->p2.key_left   = 0;
        if (sym == SDLK_RIGHT) jeu->p2.key_right  = 0;
        if (sym == SDLK_UP)    jeu->p2.key_jump   = 0;
        if (sym == SDLK_DOWN)  jeu->p2.key_defend = 0;
    }
}

/* -------------------------------------------------------
 * Game_update
 * ------------------------------------------------------- */
void Game_update(Game * jeu, float dt) {
    int i;
    if (jeu->ecran_actif != SCREEN_FIGHT) return;

    /* Timer */
    jeu->timer_acc += dt;
    if (jeu->timer_acc >= 1.0f) {
        jeu->timer_acc -= 1.0f;
        jeu->timer--;
        if (jeu->timer < 0) jeu->timer = 0;
    }

    /* Joueurs */
    Player_update(&jeu->p1, dt, &jeu->p2);
    Player_update(&jeu->p2, dt, &jeu->p1);

    /* IA : tir */
    if (jeu->p2.est_ia && jeu->p2.etat != STATE_DEAD
        && jeu->p2.cooldown_atk <= 0.f) {
        float dist = jeu->p1.x - jeu->p2.x;
        float ad   = (dist < 0.f) ? -dist : dist;
        if (ad < 360.f) {
            if (jeu->p2.energie >= ENERGIE_COST && rand() % 5 == 0)
                tirer(jeu, &jeu->p2, 1);
            else if (rand() % 3 == 0)
                tirer(jeu, &jeu->p2, 0);
        }
    }

    /* Projectiles + collisions AABB */
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile * pr = &jeu->projectiles[i];
        if (!pr->actif) continue;

        Projectile_update(pr, dt);

        if (pr->owner == 1 && Player_est_vivant(&jeu->p1)) {
            if (Projectile_collision(pr, jeu->p1.x, jeu->p1.y,
                                     jeu->p1.w, jeu->p1.h)) {
                Player_prendre_coup(&jeu->p1, pr->degats, pr->est_special);
                pr->actif = 0;
            }
        }
        if (pr->owner == 0 && Player_est_vivant(&jeu->p2)) {
            if (Projectile_collision(pr, jeu->p2.x, jeu->p2.y,
                                     jeu->p2.w, jeu->p2.h)) {
                Player_prendre_coup(&jeu->p2, pr->degats, pr->est_special);
                pr->actif = 0;
            }
        }
    }

    /* Fin du combat */
    if (!Player_est_vivant(&jeu->p1) || !Player_est_vivant(&jeu->p2)
        || jeu->timer <= 0)
        jeu->ecran_actif = SCREEN_RESULT;
}

/* -------------------------------------------------------
 * HUD (barres de vie et energie)
 * ------------------------------------------------------- */
static void render_hud(Game * jeu, SDL_Surface * ecran) {
    char buf[16];
    SDL_Rect hud = {0, 0, SCREEN_W, 55};
    SDL_FillRect(ecran, &hud, SDL_MapRGB(ecran->format, 15, 15, 25));
    hlineRGBA(ecran, 0, SCREEN_W, 55, 80, 80, 100, 255);

    /* P1 */
    stringRGBA(ecran, 10, 8, PERSONNAGES[jeu->p1.id].nom,
               jeu->p1.r, jeu->p1.g, jeu->p1.b, 255);
    stringRGBA(ecran, 10, 20, "HP", 200, 80, 80, 255);
    boxRGBA(ecran, 30, 20, 370, 31, 30, 30, 30, 255);
    {
        int w = (jeu->p1.hp * 340) / jeu->p1.hp_max;
        if (w > 0) boxRGBA(ecran, 30, 20, 30 + w, 31, 220, 60, 60, 255);
    }
    rectangleRGBA(ecran, 30, 20, 370, 31, 180, 180, 180, 200);

    stringRGBA(ecran, 10, 35, "EN", 180, 160, 40, 255);
    boxRGBA(ecran, 30, 35, 370, 46, 30, 30, 30, 255);
    {
        int w = (int)(jeu->p1.energie / jeu->p1.energie_max * 340.f);
        if (w > 0) boxRGBA(ecran, 30, 35, 30 + w, 46, 200, 160, 40, 255);
    }
    rectangleRGBA(ecran, 30, 35, 370, 46, 180, 180, 180, 200);

    /* Timer */
    sprintf(buf, "%02d", jeu->timer);
    {
        Uint8 tr = (jeu->timer <= 10) ? 255 : 200;
        Uint8 tg = (jeu->timer <= 10) ?  80 : 200;
        stringRGBA(ecran, SCREEN_W / 2 - 4, 20, buf, tr, tg, 60, 255);
    }

    /* P2 */
    {
        int nx = SCREEN_W - 10 - (int)strlen(PERSONNAGES[jeu->p2.id].nom) * 8;
        stringRGBA(ecran, nx, 8, PERSONNAGES[jeu->p2.id].nom,
                   jeu->p2.r, jeu->p2.g, jeu->p2.b, 255);
    }
    stringRGBA(ecran, SCREEN_W - 26, 20, "HP", 60, 100, 180, 255);
    boxRGBA(ecran, SCREEN_W - 370, 20, SCREEN_W - 30, 31, 30, 30, 30, 255);
    {
        int w = (jeu->p2.hp * 340) / jeu->p2.hp_max;
        if (w > 0) boxRGBA(ecran, SCREEN_W - 30 - w, 20, SCREEN_W - 30, 31,
                           60, 100, 200, 255);
    }
    rectangleRGBA(ecran, SCREEN_W - 370, 20, SCREEN_W - 30, 31, 180, 180, 180, 200);

    stringRGBA(ecran, SCREEN_W - 26, 35, "EN", 60, 160, 100, 255);
    boxRGBA(ecran, SCREEN_W - 370, 35, SCREEN_W - 30, 46, 30, 30, 30, 255);
    {
        int w = (int)(jeu->p2.energie / jeu->p2.energie_max * 340.f);
        if (w > 0) boxRGBA(ecran, SCREEN_W - 30 - w, 35, SCREEN_W - 30, 46,
                           40, 160, 100, 255);
    }
    rectangleRGBA(ecran, SCREEN_W - 370, 35, SCREEN_W - 30, 46, 180, 180, 180, 200);
}

/* -------------------------------------------------------
 * Arene
 * ------------------------------------------------------- */
static void render_arena(Game * jeu, SDL_Surface * ecran) {
    int i;
    SDL_Rect fond = {0, 55, SCREEN_W, SCREEN_H - 55};
    SDL_FillRect(ecran, &fond, SDL_MapRGB(ecran->format, 12, 12, 22));

    /* Sol */
    boxRGBA(ecran, 0, GROUND_Y, SCREEN_W, SCREEN_H, 25, 20, 35, 255);
    boxRGBA(ecran, 0, GROUND_Y, SCREEN_W, GROUND_Y + 3, 80, 60, 120, 255);
    for (i = 0; i < SCREEN_W; i += 60)
        vlineRGBA(ecran, i, GROUND_Y, SCREEN_H, 40, 30, 55, 100);

    /* Projectiles */
    for (i = 0; i < MAX_PROJECTILES; i++)
        Projectile_render(&jeu->projectiles[i], ecran);

    /* Joueurs */
    Player_render(&jeu->p1, ecran);
    Player_render(&jeu->p2, ecran);
}

/* -------------------------------------------------------
 * Game_render
 * ------------------------------------------------------- */
void Game_render(Game * jeu, SDL_Surface * ecran) {
    switch (jeu->ecran_actif) {
        case SCREEN_MENU:
            Menu_render_principal(jeu, ecran);
            break;
        case SCREEN_SELECT:
            Menu_render_selection(jeu, ecran);
            break;
        case SCREEN_FIGHT:
            render_arena(jeu, ecran);
            render_hud(jeu, ecran);
            break;
        case SCREEN_RESULT:
            render_arena(jeu, ecran);
            render_hud(jeu, ecran);
            boxRGBA(ecran, 0, 0, SCREEN_W, SCREEN_H, 0, 0, 0, 160);
            Menu_render_resultat(jeu, ecran);
            break;
    }
}

void Game_free(Game * jeu) { (void)jeu; }