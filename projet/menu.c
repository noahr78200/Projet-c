/*
 * menu.c
 */
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <string.h>
#include <stdio.h>
#include "menu.h"
#include "game.h"
#include "player.h"

/* Utilitaire : texte centre */
static void texte_centre(SDL_Surface * ecran, const char * txt,
                         int y, Uint8 r, Uint8 g, Uint8 b) {
    int x = (SCREEN_W - (int)strlen(txt) * 8) / 2;
    if (x < 0) x = 0;
    stringRGBA(ecran, x, y, txt, r, g, b, 255);
}

/* Utilitaire : barre coloree */
static void barre(SDL_Surface * ecran, int x, int y, int larg, int haut,
                  float val, float max, Uint8 r, Uint8 g, Uint8 b) {
    int rempli = (max > 0.f) ? (int)(val / max * (float)larg) : 0;
    boxRGBA(ecran, x, y, x + larg, y + haut, 30, 30, 30, 255);
    if (rempli > 0) boxRGBA(ecran, x, y, x + rempli, y + haut, r, g, b, 255);
    rectangleRGBA(ecran, x, y, x + larg, y + haut, 160, 160, 160, 200);
}

/* -------------------------------------------------------
 * Menu principal
 * ------------------------------------------------------- */
void Menu_render_principal(Game * jeu, SDL_Surface * ecran) {
    const char * options[3] = {
        "1 - JOUEUR VS JOUEUR",
        "2 - JOUEUR VS IA",
        "3 - QUITTER"
    };
    int i;

    SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 10, 10, 20));

    texte_centre(ecran, "ESGI FIGHTER",        50, 255, 80,  80);
    texte_centre(ecran, "PROTOTYPE  SDL 1.2",  68, 100, 100, 150);
    hlineRGBA(ecran, SCREEN_W/4, 3*SCREEN_W/4, 92, 100, 100, 100, 200);

    for (i = 0; i < 3; i++) {
        int y = 150 + i * 50;
        Uint8 r = 180, g = 180, b = 180;
        if (jeu->p1_selection == i) { r = 255; g = 220; b = 60; }
        texte_centre(ecran, options[i], y, r, g, b);
    }

    texte_centre(ecran, "HAUT/BAS naviguer   ENTREE valider",
                 SCREEN_H - 70, 70, 70, 70);

    if (jeu->secret_unlocked)
        texte_centre(ecran, "*** NULL_PTR DEBLOQUE ***",
                     SCREEN_H - 40, 255, 255, 60);
    else
        texte_centre(ecran, "UNE SEQUENCE SECRETE EXISTE...",
                     SCREEN_H - 40, 40, 40, 55);
}

/* -------------------------------------------------------
 * Selection des personnages
 * ------------------------------------------------------- */
void Menu_render_selection(Game * jeu, SDL_Surface * ecran) {
    int nb  = jeu->secret_unlocked ? NB_CHARS : NB_CHARS - 1;
    int colw= SCREEN_W / 4;
    int i;

    SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 8, 8, 18));

    if (jeu->select_phase == 1)
        texte_centre(ecran, "JOUEUR 1 : CHOISISSEZ",  14, 255, 100, 100);
    else
        texte_centre(ecran, "JOUEUR 2 : CHOISISSEZ",  14, 100, 150, 255);

    for (i = 0; i < nb; i++) {
        CharacterStats * s = &PERSONNAGES[i];
        int col = i % 4;
        int row = i / 4;
        int x   = col * colw + 8;
        int y   = 40 + row * 175;
        int x2  = x + colw - 12;
        int y2  = y + 165;
        int sel = (jeu->select_phase == 1 && jeu->p1_selection == i)
               || (jeu->select_phase == 2 && jeu->p2_selection == i);
        Uint8 cr = sel ? 255 : 50;
        Uint8 cg = sel ? 220 : 50;
        Uint8 cb = sel ?  60 : 70;

        rectangleRGBA(ecran, x, y, x2, y2, cr, cg, cb, 255);

        /* Mini perso */
        int mx = x + (colw - 12) / 2;
        int my = y + 12;
        filledCircleRGBA(ecran, mx, my + 10, 12, s->r, s->g, s->b, 255);
        boxRGBA(ecran, mx-11, my+22, mx+11, my+55, s->r, s->g, s->b, 255);
        boxRGBA(ecran, mx-10, my+56, mx- 2, my+78, s->r, s->g, s->b, 255);
        boxRGBA(ecran, mx+ 2, my+56, mx+10, my+78, s->r, s->g, s->b, 255);

        /* Nom */
        stringRGBA(ecran, x + 5, y + 88, s->nom, 220, 220, 220, 255);

        /* Stats */
        stringRGBA(ecran, x+5, y+100, "HP", 200, 80, 80, 255);
        barre(ecran, x+22, y+100, colw-38, 5,
              (float)s->hp_max, 150.f, 200, 60, 60);
        stringRGBA(ecran, x+5, y+111, "AT", 200,160, 40, 255);
        barre(ecran, x+22, y+111, colw-38, 5,
              (float)s->atk, 99.f, 200,160, 40);
        stringRGBA(ecran, x+5, y+122, "SP", 60, 200,120, 255);
        barre(ecran, x+22, y+122, colw-38, 5,
              s->vitesse, 270.f, 60, 200,120);

        /* Indicateur selectionne */
        if (jeu->p1_selection == i && jeu->select_phase >= 1)
            stringRGBA(ecran, x+5, y+137, "P1", 255,100,100,255);
        if (jeu->p2_selection == i && jeu->select_phase == 2)
            stringRGBA(ecran, x+22, y+137, "P2", 100,150,255,255);
    }

    texte_centre(ecran, "FLECHES naviguer   ENTREE valider   ECHAP retour",
                 SCREEN_H - 20, 70, 70, 70);
}

/* -------------------------------------------------------
 * Ecran resultat
 * ------------------------------------------------------- */
void Menu_render_resultat(Game * jeu, SDL_Surface * ecran) {
    int gagnant;

    if (!Player_est_vivant(&jeu->p1) && !Player_est_vivant(&jeu->p2))
        gagnant = 0;
    else if (!Player_est_vivant(&jeu->p2)
             || (jeu->timer <= 0 && jeu->p1.hp > jeu->p2.hp))
        gagnant = 1;
    else
        gagnant = 2;

    if (gagnant == 0) {
        texte_centre(ecran, "EGALITE !", 140, 200, 200, 60);
    } else if (gagnant == 1) {
        char buf[32];
        sprintf(buf, "%s  WINS !", PERSONNAGES[jeu->p1.id].nom);
        texte_centre(ecran, buf, 140, 255, 100, 100);
    } else {
        char buf[32];
        sprintf(buf, "%s  WINS !", PERSONNAGES[jeu->p2.id].nom);
        texte_centre(ecran, buf, 140, 100, 150, 255);
    }

    texte_centre(ecran, "K . O .", 165, 255, 220, 60);
    hlineRGBA(ecran, SCREEN_W/4, 3*SCREEN_W/4, 190, 100, 100, 100, 200);
    texte_centre(ecran, "R - REJOUER", 210, 180, 180, 180);
    texte_centre(ecran, "ECHAP - MENU", 235, 180, 180, 180);
}

/* -------------------------------------------------------
 * Clavier menu principal
 * ------------------------------------------------------- */
void Menu_key_principal(Game * jeu, SDLKey sym) {
    switch (sym) {
        case SDLK_UP:
            jeu->p1_selection--;
            if (jeu->p1_selection < 0) jeu->p1_selection = 2;
            break;
        case SDLK_DOWN:
            jeu->p1_selection++;
            if (jeu->p1_selection > 2) jeu->p1_selection = 0;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (jeu->p1_selection == 0) {
                jeu->mode         = MODE_PVP;
                jeu->select_phase = 1;
                jeu->p1_selection = 0;
                jeu->p2_selection = 1;
                jeu->ecran_actif  = SCREEN_SELECT;
            } else if (jeu->p1_selection == 1) {
                jeu->mode         = MODE_PVAI;
                jeu->select_phase = 1;
                jeu->p1_selection = 0;
                jeu->ecran_actif  = SCREEN_SELECT;
            } else {
                /* Quitter */
                SDL_Event e;
                e.type = SDL_QUIT;
                SDL_PushEvent(&e);
            }
            break;
        default: break;
    }
}

/* -------------------------------------------------------
 * Clavier selection personnage
 * ------------------------------------------------------- */
void Menu_key_selection(Game * jeu, SDLKey sym) {
    int max = jeu->secret_unlocked ? NB_CHARS - 1 : NB_CHARS - 2;
    int * sel = (jeu->select_phase == 1) ? &jeu->p1_selection
                                          : &jeu->p2_selection;
    switch (sym) {
        case SDLK_LEFT:
            (*sel)--;
            if (*sel < 0) *sel = max;
            break;
        case SDLK_RIGHT:
            (*sel)++;
            if (*sel > max) *sel = 0;
            break;
        case SDLK_UP:
            *sel -= 4;
            if (*sel < 0) *sel = 0;
            break;
        case SDLK_DOWN:
            *sel += 4;
            if (*sel > max) *sel = max;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (jeu->select_phase == 1) {
                if (jeu->mode == MODE_PVAI) {
                    jeu->p2_selection = (jeu->p1_selection + 1) % (max + 1);
                    Game_start_fight(jeu);
                    jeu->ecran_actif  = SCREEN_FIGHT;
                } else {
                    jeu->select_phase = 2;
                }
            } else {
                Game_start_fight(jeu);
                jeu->ecran_actif = SCREEN_FIGHT;
            }
            break;
        case SDLK_ESCAPE:
            jeu->ecran_actif  = SCREEN_MENU;
            jeu->p1_selection = 0;
            break;
        default: break;
    }
}