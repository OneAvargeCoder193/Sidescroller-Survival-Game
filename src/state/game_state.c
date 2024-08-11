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
int step = 0;
int playerside = 0;
int lastGrounded = 0;
float playerframe = 0;
entitystate playerstate = idle;
SDL_Texture* playerParts[6];
SDL_Texture* hat;

SDL_Texture* rainTex;

struct rain* raindrops = NULL;

void game_state_init(void) {
    playerParts[0] = shget(assets.entityTextures, "game:player_backarm");
    playerParts[1] = shget(assets.entityTextures, "game:player_backleg");
    playerParts[2] = shget(assets.entityTextures, "game:player_body");
    playerParts[3] = shget(assets.entityTextures, "game:player_frontleg");
    playerParts[4] = shget(assets.entityTextures, "game:player_head");
    playerParts[5] = shget(assets.entityTextures, "game:player_frontarm");
    hat = shget(assets.entityTextures, "game:classic_hat");

    rainTex = shget(assets.particleTextures, "game:rain");

    playery = (int)w.heightMap[(int)playerx] + 1;
}

void game_state_cleanup(void) {
    arrfree(raindrops);
}

void game_state_handle_events(SDL_Event e) {
    
}

int player_collides(float offx, float offy, float dx, float dy) {
    float minOffX = fminf(dx, 0);
    float maxOffX = fmaxf(dx, 0);
    float minOffY = fminf(dy, 0);
    float maxOffY = fmaxf(dy, 0);

    int minx = floor(playerx - 1 + offx + minOffX);
    int maxx = ceil(playerx + 1 + offx + maxOffX);
    int miny = floor(playery + offy + minOffY);
    int maxy = ceil(playery + 3 + offy + maxOffY);
    for (int y = miny; y < maxy; y++) {
        for (int x = minx; x < maxx; x++) {
            if (blocks[world_getblock(&w, x, y)].value.collision) {
                return 1;
            }
        }
    }
    return 0;
}

int move_player(float dx, float dy, int* grounded) {
    int collide = player_collides(0, 0, 0, 0);

    playery += dy;

    *grounded = 0;

    if (player_collides(0, 0, 0, 0)) {
        if (dy > 0) {
            playery = floorf(playery);
            vely = 0;
        } else if (dy < 0) {
            *grounded = 1;
            playery = ceilf(playery);
            
            vely = 0;
            if (playerstate == falling || playerstate == jumping) {
                playerframe = 0;
                playerstate = idle;
            }
        }
    }
    
    playerx += dx;

    if (player_collides(0, 0, 0, 0)) {
        if (!player_collides(0, 1, 0, 0) && *grounded) {
            playery = floorf(playery) + 1;
            vely = 0;
        } else if (dx > 0) {
            playerx = floorf(playerx);
            velx = 0;
        } else if (dx < 0) {
            playerx = ceilf(playerx);
            velx = 0;
        }
    }

    return collide;
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

    float maxSpeed = 12;

    float fric = 0.9;
    float accel = (1 - fric) * maxSpeed / fric;

    float moveX = (keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * accel;

    if (playerstate == idle) {
        if (step) {
            playerframe += 10 * delta;
            if (playerframe >= 5) {
                playerframe = 0;
                step = 0;
            }
        } else {
            step = rand() % 2048 == 0;
        }
    } else if (playerstate == walking) {
        playerframe += fabsf(moveX / accel * maxSpeed) * 4 * delta;
        if (playerframe >= 21) {
            playerframe -= 12;
        }
    } else if (playerstate == jumping) {
        playerframe = 5;
    } else if (playerstate == falling) {
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

    velx = (velx + moveX) * fric;
    vely -= 120 * delta;

    float dx = velx * delta;
    float dy = vely * delta;
    int divisions = ceil(sqrt(dx * dx + dy * dy));

    int grounded = 0;
    for (int i = 0; i < divisions; i++) {
        int g;
        if (move_player(dx / divisions, dy / divisions, &g))
            break;
        grounded = grounded || g;
    }

    if (grounded && keys[SDL_SCANCODE_SPACE]) {
        vely = sqrtf(4 * 2 * 120);
        playerstate = jumping;
        playerframe = 6;
    }

    if (!grounded && lastGrounded) {
        if (player_collides(0, -1.9, 0, 0) && !player_collides(0, -0.9, 0, 0)) {
            playery = ceilf(playery) - 1;
        }
    }

    lastGrounded = grounded;

    if (vely < 0 && playerstate != falling) {
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

    for (int i = 0; i < arrlen(raindrops); ) {
        struct rain* r = &raindrops[i];
        r->x += 20 * delta;
        r->y -= 80 * delta;

        if (blocks[world_getblock(&w, r->x, r->y)].value.collision) {
            arrdel(raindrops, i);
        } else {
            i++;
        }
    }

    for (int i = 0; i < 1; i++) {
        float xinterp = rand() / (float)RAND_MAX;

        float offset = velx * (WORLD_HEIGHT - camy + height / 32) / 80;

        int minx = -(WORLD_HEIGHT / 4) - (height / 128) + (camy / 4) - (width / 32) + camx + offset;
        int maxx = -(WORLD_HEIGHT / 4) + (height / 128) + (camy / 4) + (width / 32) + camx + offset;
        float x = (maxx - minx) * xinterp + minx;
        float y = WORLD_HEIGHT;
        arrput(raindrops, ((struct rain){x, y}));
    }

    if (left) {
        if (world_getblock(&w, x, y) != shgeti(blocks, "game:wood")) {
            world_setblockdata(&w, x, y, shgeti(blocks, "game:wood"));
            world_gendatarange(&w, x - 1, y - 1, x + 2, y + 2);
        }
    } else if (right) {
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
    camy = fmaxf(fminf(renplayery, WORLD_HEIGHT - (float)height / 32), (float)height / 32);
    
    for (int i = 0; i < arrlen(entities); i++) {
        renderEntity(renderer, entities[i], camx, camy);
    }

    int animId = (int)playerframe;
    int side = playerside;

    world_render(&w, camx, camy, blocks, renderer);

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

    for (int i = 0; i < arrlen(raindrops); i++) {
        struct rain r = raindrops[i];
        
        float renx = floorf(r.x * 8) / 8;
        float reny = floorf(r.y * 8) / 8;

        SDL_Rect rect;
        rect.x = (renx - camx) * 16 + width / 2.0 - 16;
        rect.y = height - (reny - camy) * 16 - height / 2.0 - 16;
        rect.w = 32;
        rect.h = 32;
        
        SDL_RenderCopy(renderer, rainTex, NULL, &rect);
    }
}
