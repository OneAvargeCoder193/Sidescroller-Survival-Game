#include "loading_state.h"
#include <stdio.h>

#include "state.h"
#include "world.h"

int numPerFrame;

void loading_state_init(void) {
    numPerFrame = 1000;
}

void loading_state_cleanup(void) {
    
}

void loading_state_handle_events(SDL_Event e) {
    
}

void loading_state_update(SDL_Renderer* renderer, float delta) {
    if (delta > 1 / 140.0) {
        numPerFrame--;
    } else {
        numPerFrame++;
    }

    if (w.generateState == genEmpty) {
        w = world_init(blocks);
    }
    if (w.generateState != genDone) {
        // Automatically calculate how many steps to do in one frame based on deltatime
        for (int i = 0; i < numPerFrame; i++) {
            switch (w.generateState) {
            case genBlocks:
                world_genblock(&w);
                break;
            case genPlaceLava:
                world_placelava(&w);
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

    char* text = "";
    switch (w.generateState) {
    case genBlocks:
        text = "Generating World Shape";
        break;
    case genPlaceLava:
        text = "Placing Lava";
        break;
    case genLiquids:
        text = "Filling Liquids";
        break;
    case genVegetation:
        text = "Adding Vegetation";
        break;
    case genData:
        text = "Finishing up";
        break;
    case genDone:
        text = "Done";
        break;
    }

    render_text(renderer, width / 2, height / 2 - 40, text, 48, (SDL_Color){255, 255, 255}, font, XALIGN_CENTER, YALIGN_CENTER);
}