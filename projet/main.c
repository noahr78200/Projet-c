/*
 * main.c
 * Point d'entree + boucle principale SDL
 * Compile : gcc main.c game.c player.c projectile.c menu.c -lmingw32 -lSDLmain -lSDL -lSDL_gfx -o fighter.exe
 */
#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>
#include <stdlib.h>
#include "game.h"
#include "menu.h"

int main(int argc, char * argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init erreur : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Surface * ecran = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32,
                                           SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (ecran == NULL) {
        fprintf(stderr, "SDL_SetVideoMode erreur : %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_WM_SetCaption("ESGI FIGHTER", NULL);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    Game jeu;
    Game_init(&jeu, ecran);

    int active = 1;
    SDL_Event event;
    Uint32 last  = SDL_GetTicks();
    Uint32 now;
    float  dt;

    while (active) {
        /* Evenements (chap. 17) */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    active = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE
                        && jeu.ecran_actif == SCREEN_FIGHT) {
                        jeu.ecran_actif  = SCREEN_MENU;
                        jeu.p1_selection = 0;
                    } else {
                        Game_key_down(&jeu, event.key.keysym.sym);
                    }
                    break;
                case SDL_KEYUP:
                    Game_key_up(&jeu, event.key.keysym.sym);
                    break;
                default:
                    break;
            }
        }

        /* Delta time */
        now = SDL_GetTicks();
        dt  = (float)(now - last) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        last = now;

        /* Update (logique) */
        Game_update(&jeu, dt);

        /* Render (affichage) */
        Game_render(&jeu, ecran);
        SDL_Flip(ecran);

        SDL_Delay(16);  /* ~60 fps */
    }

    Game_free(&jeu);
    SDL_FreeSurface(ecran);
    SDL_Quit();
    return EXIT_SUCCESS;
}