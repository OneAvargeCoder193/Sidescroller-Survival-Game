#include "menu_state.h"
#include <stdio.h>

#include "utils.h"

#include "state.h"

void menu_state_init(void) {
    
}

void menu_state_cleanup(void) {
    
}

void menu_state_handle_events(void) {
    
}

void menu_state_update(float delta) {

}

void menu_state_draw(SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    render_text(renderer, width / 2, 24, "-Game Title-", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_CENTER, YALIGN_BOTTOM);
    
    if (render_button(renderer, 24, height - 152 - 24, "Singleplayer", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_LEFT, YALIGN_CENTER)) {
        state_set(STATE_LOADING);
    }

    render_button(renderer, 24, height - 88 - 24, "Multiplayer", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_LEFT, YALIGN_CENTER);
    render_button(renderer, 24, height - 24 - 24, "Settings", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_LEFT, YALIGN_CENTER);
}
