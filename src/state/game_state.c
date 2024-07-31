#include "game_state.h"
#include <stdio.h>

#include "state.h"
#include "world.h"
#include "entity.h"
#include "assets.h"

#include "stb_ds.h"

float camx = 0;
float camy = 0;

float playerx = WORLD_WIDTH / 2;
float playery = WORLD_HEIGHT / 2;
int playerside = 0;
float playerframe = 0;
entitystate playerstate = idle;
SDL_Texture* playerParts[6];
SDL_Texture* hat;

void game_state_init(void) {
    playerParts[0] = shget(assets.entityTextures, "game:player_backarm");
    playerParts[1] = shget(assets.entityTextures, "game:player_backleg");
    playerParts[2] = shget(assets.entityTextures, "game:player_body");
    playerParts[3] = shget(assets.entityTextures, "game:player_frontleg");
    playerParts[4] = shget(assets.entityTextures, "game:player_head");
    playerParts[5] = shget(assets.entityTextures, "game:player_frontarm");
    hat = shget(assets.entityTextures, "game:classic_hat");
}

void game_state_cleanup(void) {
    
}

void game_state_handle_events(void) {
    
}

void game_state_update(SDL_Renderer* renderer, float delta) {
    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    int save = (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]) && keys[SDL_SCANCODE_S];
    if (save) {
        FILE* out = fopen("save.bin", "wb");
        world_save(&w, out);
        fclose(out);
    }

    int load = (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]) && keys[SDL_SCANCODE_L];
    if (load) {
        FILE* in = fopen("save.bin", "rb");
        world_load(&w, in);
        fclose(in);
    }

    for (int i = 0; i < arrlen(entities); i++) {
        updateEntity(&entities[i], delta);
    }

    float moveX = (keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * 8;
    float moveY = (keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]) * 8;

    if (playerstate == walking) {
        playerframe += fabsf(moveX) * 8 * delta;
        if (playerframe >= 21) {
            playerframe -= 12;
        }
    }

    if (moveX < 0) {
        playerside = 0;
        if (playerstate != walking)
            playerframe = 9;
        playerstate = walking;
    } else if (moveX > 0) {
        playerside = 1;
        if (playerstate != walking)
            playerframe = 9;
        playerstate = walking;
    } else if (moveX == 0) {
        if (playerstate != idle)
            playerframe = 0;
        playerstate = idle;
    }

    playerx += moveX * delta;
    playery += moveY * delta;
    
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    int x, y;
    int state = SDL_GetMouseState(&x, &y);

    int left = state & SDL_BUTTON(SDL_BUTTON_LEFT);
    int right = state & SDL_BUTTON(SDL_BUTTON_RIGHT);

    float px = x - width / 2;
    float py = height - y - height / 2;
    px /= 16;
    py /= 16;
    px += camx;
    py += camy;
    x = px;
    y = py;

    if (right) {
        if (world_getblock(&w, x, y) != shgeti(blocks, "game:wood")) {
            world_setblockdata(&w, x, y, shgeti(blocks, "game:wood"));
            world_gendatarange(&w, x - 1, y - 1, x + 2, y + 2);
        }
    } else if (left) {
        if (world_getblock(&w, x, y) != shgeti(blocks, "game:air")) {
            world_setblockdata(&w, x, y, shgeti(blocks, "game:air"));
            world_gendatarange(&w, x - 1, y - 1, x + 2, y + 2);
        }
    }
}

void game_state_draw(SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    float renplayerx = floorf(playerx * 8) / 8;
    float renplayery = floorf(playery * 8) / 8;
    
    camx = fmaxf(renplayerx, (float)width / 32);
    camy = fmaxf(renplayery, (float)height / 32);
    
    for (int i = 0; i < arrlen(entities); i++) {
        renderEntity(renderer, entities[i], camx, camy);
    }

    int animId = (int)playerframe;
    int side = playerside;

    SDL_Rect playerSrc;
    playerSrc.x = animId * 32;
    playerSrc.y = side * 32;
    playerSrc.w = 32;
    playerSrc.h = 32;

    SDL_Rect playerDst;
    playerDst.x = (renplayerx - camx) * 16 + width / 2.0 - 32;
    playerDst.y = height - (renplayery - camy) * 16 - height / 2.0 - 32;
    playerDst.w = 64;
    playerDst.h = 64;
    
    SDL_RenderCopy(renderer, playerParts[0], &playerSrc, &playerDst);
    SDL_RenderCopy(renderer, playerParts[1], &playerSrc, &playerDst);
    SDL_RenderCopy(renderer, playerParts[2], &playerSrc, &playerDst);
    SDL_RenderCopy(renderer, playerParts[3], &playerSrc, &playerDst);
    SDL_RenderCopy(renderer, playerParts[4], &playerSrc, &playerDst);
    SDL_RenderCopy(renderer, hat, &playerSrc, &playerDst);
    SDL_RenderCopy(renderer, playerParts[5], &playerSrc, &playerDst);

    world_render(&w, camx, camy, blocks, renderer);
}
