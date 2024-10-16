#ifndef LOADING_STATE_H
#define LOADING_STATE_H

#include <SDL2/SDL.h>

extern int numPerFrame;

void loading_state_init(void);
void loading_state_cleanup(void);
void loading_state_handle_events(SDL_Event e);
void loading_state_update(SDL_Renderer* renderer, float delta);
void loading_state_draw(SDL_Renderer* renderer);

#endif // LOADING_STATE_H
