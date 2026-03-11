#include <stdio.h>
#include <stdlib.h>




typedef enum PlayerState {
    STATE_VOID,
    STATE_RUN,
    STATE_JUMP,
    STATE_ATTACK,
    STATE_DEFEND,
    STATE_SPECIAL,
    STATE_HURT,
    STATE_DEAD
} PlayerState;

typedef struct Player {
    
    float x, y;       
    int   w, h;       

    
    int hp;           
    int hp_max;        
    int degats;        
    int degats_special;

    
    PlayerState etat;       
    int         etat_timer; 

    
    int direction;   
    float vx, vy; 
}


