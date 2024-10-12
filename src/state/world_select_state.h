#ifndef MENU_STATE_H
#define MENU_STATE_H

#include <SDL2/SDL.h>

#include "utils.h"

extern SDL_Texture* button_idle;
extern SDL_Texture* button_hovered;
extern SDL_Texture* button_pressed;

extern textinput textInput;

extern char** worldPaths;

void world_select_state_init(void);
void world_select_state_cleanup(void);
void world_select_state_handle_events(SDL_Event e);
void world_select_state_update(SDL_Renderer* renderer, float delta);
void world_select_state_draw(SDL_Renderer* renderer);

#endif // MENU_STATE_H