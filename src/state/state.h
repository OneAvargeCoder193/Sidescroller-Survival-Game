#ifndef STATE_H
#define STATE_H

#include <SDL2/SDL.h>

typedef enum {
    STATE_MENU,
    STATE_WORLD_SELECT,
    STATE_LOADING,
    STATE_CREDITS,
    STATE_GAME
} state_type;

extern state_type state;

void state_init(void);
void state_cleanup(void);
void state_set(state_type new_state);
void state_handle_events(SDL_Event e);
void state_update(SDL_Renderer* renderer, float delta);
void state_draw(SDL_Renderer* renderer);

#endif // STATE_H
