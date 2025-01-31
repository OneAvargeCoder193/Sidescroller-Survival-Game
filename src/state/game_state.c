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
int selectedBlock = 1;
float playerframe = 0;
float playerGroundTime = 0;
int deleteLayer = -1;
entitystate playerstate = idle;
SDL_Texture* playerParts[6];
SDL_Texture* hat;

SDL_Texture* rainTex;

SDL_Texture* sun;
SDL_Texture* sunRing;

Mix_Music* music = NULL;

struct rain* raindrops = NULL;

enum weather weatherState;
float weatherTime;

void game_state_init(void) {
    playerParts[0] = shget(assets.entityTextures, "game:player_backarm");
    playerParts[1] = shget(assets.entityTextures, "game:player_backleg");
    playerParts[2] = shget(assets.entityTextures, "game:player_body");
    playerParts[3] = shget(assets.entityTextures, "game:player_frontleg");
    playerParts[4] = shget(assets.entityTextures, "game:player_head");
    playerParts[5] = shget(assets.entityTextures, "game:player_frontarm");
    hat = shget(assets.entityTextures, "game:classic_hat");

    rainTex = shget(assets.particleTextures, "game:rain");

    sun = shget(assets.textures, "game:sun");
    sunRing = shget(assets.textures, "game:sun_ring");

    playery = (int)w.heightMap[(int)playerx] + 1;
    
    float renplayerx = floorf(playerx * 8) / 8;
    float renplayery = floorf(playery * 8) / 8;

    camx = renplayerx;
    camy = renplayery;

    weatherState = weather_clear;
    weatherTime = rand() % 4000 + 800;

    music = Mix_LoadMUS("assets/music/Will of Iron (Sibrix' theme) - Vacant Space OST.ogg");
}

void game_state_cleanup(void) {
    arrfree(raindrops);
    Mix_FreeMusic(music);
    world_deinit(&w);
}

void game_state_handle_events(SDL_Event e) {
    if (e.type == SDL_MOUSEWHEEL) {
        int scrollsign = (e.wheel.y > 0) - (e.wheel.y < 0);
        selectedBlock += scrollsign;
        if (selectedBlock >= shlen(blocks)) {
            selectedBlock = 1;
        }
        if (selectedBlock < 1) {
            selectedBlock = shlen(blocks) - 1;
        }
    }
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
    if (Mix_PlayingMusic() == 0) {
        Mix_PlayMusic(music, -1);
        Mix_VolumeMusic(32);
    }

    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    int save = (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]) && keys[SDL_SCANCODE_S];
    if (save) {
        FILE* out = fopen(w.path, "wb+");
        if (errno != 0) {
            printf("Failed to save world: %s\n", strerror(errno));
        } else {
            world_save(&w, out);
        }
        fclose(out);
    }

    // int load = (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]) && keys[SDL_SCANCODE_L];
    // if (load) {
    //     FILE* in = fopen("save.bin", "rb");
    //     world_load(&w, in);
    //     fclose(in);
    // }

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
        if ((moveX < 0 && playerside == 0) || (moveX > 0 && playerside == 1)) {
            playerframe -= fabsf(moveX / accel * maxSpeed) * 4 * delta;
            if (playerframe < 9) {
                playerframe += 12;
            }
        } else {
            playerframe += fabsf(moveX / accel * maxSpeed) * 4 * delta;
            if (playerframe >= 21) {
                playerframe -= 12;
            }
        }
    } else if (playerstate == jumping) {
        playerframe = 5;
    } else if (playerstate == falling) {
        playerframe += 6 * delta;
        if (playerframe >= 9) {
            playerframe = 8;
        }
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

    playerGroundTime += delta;

    int grounded = 0;
    for (int i = 0; i < divisions; i++) {
        int g;
        if (move_player(dx / divisions, dy / divisions, &g))
            break;
        grounded = grounded || g;
        if (g) {
            playerGroundTime = 0;
        }
    }

    if (playerGroundTime < 0.05 && keys[SDL_SCANCODE_SPACE]) {
        vely = sqrtf(4 * 2 * 120);
        playerstate = jumping;
        playerframe = 6;
        playerGroundTime = 1000;
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
    
    float renplayerx = floorf(playerx * 8) / 8;
    int playerScreenX = (renplayerx - camx) * 16 + width / 2.0 - 32;

    if (moveX != 0) {
        if (x < playerScreenX) {
            playerside = 1;
        } else {
            playerside = 0;
        }
    }

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
        r->frame += delta * 24;

        if (r->frame > 7) {
            arrdel(raindrops, i);
        } else {
            i++;
        }
    }

    weatherTime -= delta;
    if (weatherTime < 0) {
        enum weather old = weatherState;
        
        weatherState = rand() % 2;
        while (weatherState == old) {
            weatherState = rand() % 2;
        }

        weatherTime = rand() % 4000 + 800;
    }

    // for (int i = 0; i < 1; i++) {
    if (weatherState == weather_raining && rand() % 256 < 128) {
        float xinterp = rand() / (float)RAND_MAX;

        int minx = camx - width / 32;
        int maxx = camx + width / 32;
        float x = (maxx - minx) * xinterp + minx;
        float y = WORLD_HEIGHT;
        while (!blocks[world_getblock(&w, x, y - 1)].value.collision) {
            y--;
        }
        arrput(raindrops, ((struct rain){x, y, 0}));
    }
    // }

    if (left) {
        int layer = blocks[selectedBlock].value.layer;
        if (world_getblocklayer(&w, x, y, layer) != selectedBlock) {
            world_setblockanddatalayer(&w, x, y, layer, selectedBlock);
            world_gendatarange(&w, x - 1, y - 1, x + 2, y + 2);
        }
    }
    
    if (right) {
        if (deleteLayer == -1) {
            if (world_getblock(&w, x, y) != shgeti(blocks, "game:air")) {
                deleteLayer = WORLD_LAYER;
            } else if (world_getwall(&w, x, y) != shgeti(blocks, "game:air")) {
                deleteLayer = WALL_LAYER;
            }
        }
        
        if (deleteLayer != -1) {
            if ((world_getblock(&w, x, y) == shgeti(blocks, "game:air") != (deleteLayer == WORLD_LAYER)) && world_getblocklayer(&w, x, y, deleteLayer) != shgeti(blocks, "game:air")) {
                world_setblockanddatalayer(&w, x, y, deleteLayer, shgeti(blocks, "game:air"));
                world_gendatarange(&w, x - 1, y - 1, x + 2, y + 2);
            }
        }
    } else {
        deleteLayer = -1;
    }

    if (arrlen(w.updateLightingPos) != 0) {
        world_genlight(&w);
    }
}

float calculate_sun_coverage(SDL_Rect r, int width, int height) {
    int num = 0;
    int covered = 0;

    for (int y = r.y + 8; y < r.y + r.h; y += 16) {
        for (int x = r.x + 8; x < r.x + r.w; x += 16) {
            int cx = x - r.x - r.w / 2;
            int cy = y - r.y - r.h / 2;

            int dist = cx * cx + cy * cy;

            if (dist > 900)
                continue;

            int px = (int)((x - width / 2) / 16.0 + camx);
            int py = (int)((height - y - height / 2) / 16.0 + camy);
            
            num++;

            if (blocks[world_getblock(&w, px, py)].value.collision) {
                covered++;
            }
        }
    }

    return (float)covered / num;
}

void game_state_draw(SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    float renplayerx = floorf(playerx * 8) / 8;
    float renplayery = floorf(playery * 8) / 8;
    
    camx += (fmaxf(fminf(renplayerx, WORLD_WIDTH - (float)width / 32), (float)width / 32) - camx) / 5.0;
    camy += (fmaxf(fminf(renplayery, WORLD_HEIGHT - (float)height / 32), (float)height / 32) - camy) / 5.0;

    SDL_Rect sunDst;
    sunDst.x = width / 2 - 128;
    sunDst.y = height / 4 - 128;
    sunDst.w = 256;
    sunDst.h = 256;

    SDL_RenderCopy(renderer, sun, NULL, &sunDst);
    
    float sunCoverage = calculate_sun_coverage(sunDst, width, height);
    sunCoverage = sqrtf(1 - sunCoverage);
    SDL_SetTextureAlphaMod(sunRing, sunCoverage * 255);
    SDL_RenderCopy(renderer, sunRing, NULL, &sunDst);
    SDL_SetTextureAlphaMod(sunRing, 255);

    world_render(&w, camx, camy, blocks, renderer);

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

    for (int i = 0; i < arrlen(entities); i++) {
        renderEntity(renderer, entities[i], camx, camy);
    }

    for (int i = 0; i < arrlen(raindrops); i++) {
        struct rain r = raindrops[i];
        
        float renx = floorf(r.x * 8) / 8;
        float reny = floorf(r.y * 8) / 8;

        SDL_Rect src;
        src.x = floorf(r.frame) * 16;
        src.y = 0;
        src.w = 16;
        src.h = 16;

        SDL_Rect rect;
        rect.x = (renx - camx) * 16 + width / 2.0 - 16;
        rect.y = height - (reny - camy) * 16 - height / 2.0 - 32;
        rect.w = 32;
        rect.h = 32;
        
        SDL_RenderCopy(renderer, rainTex, &src, &rect);
    }

    if (weatherState == weather_raining) {
        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 64);
        SDL_RenderFillRect(renderer, NULL);
    }

    int data = 0;

    if (blocks[selectedBlock].value.shape == shape_edges) {
        data = 0xFFFF; // 0b1111111111111111
    }

    SDL_Texture* selectedBlockTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 32, 32);
    SDL_SetTextureBlendMode(selectedBlockTex, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(renderer, selectedBlockTex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    world_drawBlockPos(renderer, blocks, selectedBlock | data << 8, 0xff, 0, 0, 8, 8);
    world_drawBlockFoliagePos(renderer, blocks, selectedBlock | data << 8, 0xff, 0, 0, 8, 8);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_Rect selectedDst;
    selectedDst.x = 8;
    selectedDst.y = 8;
    selectedDst.w = 64;
    selectedDst.h = 64;

    SDL_RenderCopy(renderer, selectedBlockTex, NULL, &selectedDst);
}