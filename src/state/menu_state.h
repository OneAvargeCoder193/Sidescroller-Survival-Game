#ifndef MENU_STATE_H
#define MENU_STATE_H

#include <SDL2/SDL.h>

void menu_state_init(void);
void menu_state_cleanup(void);
void menu_state_handle_events(SDL_Event e);
void menu_state_update(SDL_Renderer* renderer, float delta);
void menu_state_draw(SDL_Renderer* renderer);

#endif // MENU_STATE_H