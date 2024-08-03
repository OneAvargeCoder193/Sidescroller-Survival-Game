#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <SDL2/SDL.h>

typedef enum entitystate {
    idle,
    walking,
    jumping,
    falling
} entitystate;

extern float camx;
extern float camy;

extern float playerx;
extern float playery;
extern float velx;
extern float vely;
extern int playerside;
extern float playerframe;
extern entitystate playerstate;
extern SDL_Texture* playerParts[6];
extern SDL_Texture* hat;

void game_state_init(void);
void game_state_cleanup(void);
void game_state_handle_events(void);
void game_state_update(SDL_Renderer* renderer, float delta);
void game_state_draw(SDL_Renderer* renderer);

#endif // GAME_STATE_H