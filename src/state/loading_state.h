#ifndef LOADING_STATE_H
#define LOADING_STATE_H

#include <SDL2/SDL.h>

void loading_state_init(void);
void loading_state_cleanup(void);
void loading_state_handle_events(void);
void loading_state_update(float delta);
void loading_state_draw(SDL_Renderer* renderer);

#endif // LOADING_STATE_H
