#include "state.h"
#include <stdio.h>

state_type state = STATE_MENU;

// Forward declarations of state functions
void menu_state_init(void);
void menu_state_cleanup(void);
void menu_state_handle_events(void);
void menu_state_update(SDL_Renderer* renderer, float delta);
void menu_state_draw(SDL_Renderer* renderer);

void loading_state_init(void);
void loading_state_cleanup(void);
void loading_state_handle_events(void);
void loading_state_update(SDL_Renderer* renderer, float delta);
void loading_state_draw(SDL_Renderer* renderer);

void game_state_init(void);
void game_state_cleanup(void);
void game_state_handle_events(void);
void game_state_update(SDL_Renderer* renderer, float delta);
void game_state_draw(SDL_Renderer* renderer);

void state_init(void) {
    state = STATE_MENU;
}

void state_set(state_type new_state) {
    // Clean up current state
    switch (state) {
        case STATE_MENU:
            menu_state_cleanup();
            break;
        case STATE_LOADING:
            loading_state_cleanup();
            break;
        case STATE_GAME:
            game_state_cleanup();
            break;
        default:
            break;
    }

    // Initialize new state
    state = new_state;
    switch (state) {
        case STATE_MENU:
            menu_state_init();
            break;
        case STATE_LOADING:
            loading_state_init();
            break;
        case STATE_GAME:
            game_state_init();
            break;
        default:
            break;
    }
}

void state_update(SDL_Renderer* renderer, float delta) {
    switch (state) {
        case STATE_MENU:
            menu_state_handle_events();
            menu_state_update(renderer, delta);
            break;
        case STATE_LOADING:
            loading_state_handle_events();
            loading_state_update(renderer, delta);
            break;
        case STATE_GAME:
            game_state_handle_events();
            game_state_update(renderer, delta);
            break;
        default:
            break;
    }
}

void state_draw(SDL_Renderer* renderer) {
    switch (state) {
        case STATE_MENU:
            menu_state_draw(renderer);
            break;
        case STATE_LOADING:
            loading_state_draw(renderer);
            break;
        case STATE_GAME:
            game_state_draw(renderer);
            break;
        default:
            break;
    }
}
