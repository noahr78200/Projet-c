/*
 * player.h
 */
#ifndef DEF_PLAYER_H
#define DEF_PLAYER_H

#include <SDL/SDL.h>

/* enum PlayerState : machine a etats */
typedef enum {
    STATE_IDLE,
    STATE_RUN,
    STATE_JUMP,
    STATE_ATTACK,
    STATE_DEFEND,
    STATE_SPECIAL,
    STATE_HURT,
    STATE_DEAD
} PlayerState;

/* struct CharacterStats */
typedef struct {
    const char * nom;
    int    hp_max;
    int    atk;
    int    atk_special;
    float  vitesse;
    float  energie_rate;
    Uint8  r, g, b;
    Uint8  pr, pg, pb;
} CharacterStats;

#define NB_CHARS 7
extern CharacterStats PERSONNAGES[NB_CHARS];

/* struct Player */
typedef struct {
    float  x, y;
    int    w, h;
    float  vx, vy;
    int    au_sol;
    int    face;        /* 1 = droite, -1 = gauche */

    int    hp, hp_max;
    int    atk, atk_special;
    float  vitesse;
    float  energie_rate;
    float  energie, energie_max;

    PlayerState etat;
    float       etat_timer;
    float       cooldown_atk;
    int         hit_flash;

    int    key_left, key_right, key_jump;
    int    key_defend;

    int    est_ia;
    float  ia_timer;

    int    id;          /* 0 = P1, 1 = P2 */
    Uint8  r, g, b;
    Uint8  pr, pg, pb;
} Player;

void Player_init        (Player * p, int id, int char_id);
void Player_update      (Player * p, float dt, Player * adversaire);
void Player_render      (Player * p, SDL_Surface * ecran);
void Player_prendre_coup(Player * p, int degats, int est_special);
int  Player_est_vivant  (const Player * p);
void Player_update_ia   (Player * p, Player * ennemi, float dt);

#endif