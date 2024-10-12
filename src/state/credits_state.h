#ifndef CREDITS_STATE_H
#define CREDITS_STATE_H

#include <SDL2/SDL.h>

extern FILE* creditsFp;

void credits_state_init(void);
void credits_state_cleanup(void);
void credits_state_handle_events(SDL_Event e);
void credits_state_update(SDL_Renderer* renderer, float delta);
void credits_state_draw(SDL_Renderer* renderer);

#endif // CREDITS_STATE_H