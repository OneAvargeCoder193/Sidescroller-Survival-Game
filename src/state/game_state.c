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
float velx = 0;
float vely = 0;
int playerside = 0;
int lastGrounded = 0;
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

    playery = (int)w.heightMap[(int)playerx] + 1;
}

void game_state_cleanup(void) {
    
}

void game_state_handle_events(void) {
    
}

int player_collides(float offx, float offy) {
    int minx = floor(playerx - 1 + offx);
    int maxx = ceil(playerx + 1 + offx);
    int miny = floor(playery + offy);
    int maxy = ceil(playery + 3 + offy);
    for (int y = miny; y < maxy; y++) {
        for (int x = minx; x < maxx; x++) {
            if (world_getblock(&w, x, y) != 0) {
                return 1;
            }
        }
    }
    return 0;
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

    float moveX = (keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * 12;

    if (playerstate == walking) {
        playerframe += fabsf(moveX) * 4 * delta;
        if (playerframe >= 21) {
            playerframe -= 12;
        }
    }
    if (playerstate == jumping) {
        playerframe = 5;
    }
    if (playerstate == falling) {
        playerframe += 6 * delta;
        if (playerframe >= 9) {
            playerframe = 8;
        }
    }

    if (moveX < 0) {
        playerside = 1;
    } else if (moveX > 0) {
        playerside = 0;
    }

    if (playerstate == walking || playerstate == idle) {
        if (moveX < 0) {
            if (playerstate != walking)
                playerframe = 9;
            playerstate = walking;
        } else if (moveX > 0) {
            if (playerstate != walking)
                playerframe = 9;
            playerstate = walking;
        } else if (moveX == 0) {
            if (playerstate != idle)
                playerframe = 0;
            playerstate = idle;
        }
    }

    velx = moveX;
    vely -= 120 * delta;

    playerx += velx * delta;

    if (player_collides(0, 0)) {
        if (!player_collides(0, 1)) {
            playery = floorf(playery) + 1;
            vely = 0;
        } else if (velx > 0) {
            playerx = floorf(playerx);
            velx = 0;
        } else if (velx < 0) {
            playerx = ceilf(playerx);
            velx = 0;
        }
    }

    playery += vely * delta;

    int grounded = 0;

    if (player_collides(0, 0)) {
        if (vely > 0) {
            playery = floorf(playery);
            vely = 0;
        } else if (vely < 0) {
            grounded = 1;
            playery = ceilf(playery);
            if (keys[SDL_SCANCODE_SPACE]) {
                vely = sqrtf(4 * 2 * 120);
                playerstate = jumping;
            } else {
                vely = 0;
                if (playerstate == falling || playerstate == jumping) {
                    playerframe = 0;
                    playerstate = idle;
                }
            }
        }
    }

    if (!grounded && lastGrounded) {
        if (player_collides(0, -2) && !player_collides(0, -0.9)) {
            playery = ceilf(playery) - 1;
        }
    }

    lastGrounded = grounded;

    if (vely < 0) {
        playerstate = falling;
        playerframe = 7;
    }

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
    
    camx = fmaxf(fminf(renplayerx, WORLD_WIDTH - (float)width / 32), (float)width / 32);
    camy = fmaxf(fminf(renplayery, WORLD_WIDTH - (float)width / 32), (float)height / 32);
    
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
    playerDst.y = height - (renplayery - camy) * 16 - height / 2.0 - 64;
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
