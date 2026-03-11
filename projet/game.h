/*
 * game.h
 */
#ifndef DEF_GAME_H
#define DEF_GAME_H

#include <SDL/SDL.h>
#include "player.h"
#include "projectile.h"

#define SCREEN_W        800
#define SCREEN_H        500
#define GROUND_Y        400
#define MAX_PROJECTILES  16
#define ENERGIE_COST    60.f
#define TIMER_COMBAT     60

typedef enum {
    SCREEN_MENU,
    SCREEN_SELECT,
    SCREEN_FIGHT,
    SCREEN_RESULT
} GameScreen;

typedef enum {
    MODE_PVP,
    MODE_PVAI
} GameMode;

/* struct Game */
typedef struct {
    GameScreen  ecran_actif;
    GameMode    mode;

    Player      p1;
    Player      p2;

    Projectile  projectiles[MAX_PROJECTILES];

    int         timer;
    float       timer_acc;

    int         p1_selection;
    int         p2_selection;
    int         select_phase;   /* 1 = P1 choisit, 2 = P2 choisit */

    int         secret_buffer[8];
    int         secret_len;
    int         secret_unlocked;
} Game;

void Game_init       (Game * jeu, SDL_Surface * ecran);
void Game_start_fight(Game * jeu);
void Game_update     (Game * jeu, float dt);
void Game_render     (Game * jeu, SDL_Surface * ecran);
void Game_free       (Game * jeu);
void Game_key_down   (Game * jeu, SDLKey sym);
void Game_key_up     (Game * jeu, SDLKey sym);

#endif