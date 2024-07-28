#include "loading_state.h"
#include <stdio.h>

#include "state.h"
#include "world.h"

void loading_state_init(void) {
    
}

void loading_state_cleanup(void) {
    
}

void loading_state_handle_events(void) {
    
}

void loading_state_update(SDL_Renderer* renderer, float delta) {
    if (w.generateState == genEmpty) {
        w = world_init(blocks);
    }
    if (w.generateState != genDone) {
        for (int i = 0; i < 1000; i++) {
            switch (w.generateState) {
            case genBlocks:
                world_genblock(&w);
                break;
            case genLiquids:
                world_fillliquids(&w);
                break;
            case genData:
                world_gendata(&w);
                break;
            case genDone:
                break;
            }
        }
    } else {
        state_set(STATE_GAME);
    }
}

void loading_state_draw(SDL_Renderer* renderer) {
    
}
