/*
 * menu.h
 */
#ifndef DEF_MENU_H
#define DEF_MENU_H

#include <SDL/SDL.h>
#include "game.h"

void Menu_render_principal(Game * jeu, SDL_Surface * ecran);
void Menu_render_selection(Game * jeu, SDL_Surface * ecran);
void Menu_render_resultat (Game * jeu, SDL_Surface * ecran);
void Menu_key_principal   (Game * jeu, SDLKey sym);
void Menu_key_selection   (Game * jeu, SDLKey sym);

#endif