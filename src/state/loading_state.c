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
            case genVegetation:
                world_addvegetation(&w);
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
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect outlineRect;
    outlineRect.x = width / 2 - 200;
    outlineRect.y = height / 2 - 25;
    outlineRect.w = 400;
    outlineRect.h = 50;
    SDL_RenderFillRect(renderer, &outlineRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect insideRect;
    insideRect.x = width / 2 - 195;
    insideRect.y = height / 2 - 20;
    insideRect.w = 390;
    insideRect.h = 40;
    SDL_RenderFillRect(renderer, &insideRect);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect progressRect;
    progressRect.x = width / 2 - 195;
    progressRect.y = height / 2 - 20;
    progressRect.w = 390 * (float)w.genIdx / (WORLD_WIDTH * WORLD_HEIGHT);
    progressRect.h = 40;
    SDL_RenderFillRect(renderer, &progressRect);
}
