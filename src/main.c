#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "utils.h"
#include "world.h"
#include "assets.h"

world w;
SDL_Renderer *renderer;
// block *blocks = NULL;
SDL_Texture* playerParts[6];
SDL_Texture* hat;

Assets assets;

float camx = 0;
float camy = 0;

float playerx = WORLD_WIDTH / 2;
float playery = WORLD_HEIGHT / 2;
int playerside = 0;
float playerframe = 5;
typedef enum entitystate {
    idle,
    walking
} entitystate;
entitystate playerstate;

void render() {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    float renplayerx = floorf(playerx * 8) / 8;
    float renplayery = floorf(playery * 8) / 8;
    
    camx = fmaxf(renplayerx, (float)width / 32);
    camy = fmaxf(renplayery, (float)height / 32);

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

    world_render(w, camx, camy, blocks, renderer);
}

SDL_Texture* loadTexture(const char* path) {
    return IMG_LoadTexture(renderer, path);
}

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize the SDL2 library\n");
        printf("SDL2 Error: %d\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Survival Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1280, 720,
                                          0);
    
    if(!window)
    {
        printf("Failed to create window\n");
        printf("SDL2 Error: %d\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(!renderer)
    {
        printf("Failed to create the renderer\n");
        printf("SDL2 Error: %d\n", SDL_GetError());
        return -1;
    }

    init_assets(&assets, renderer);

    SDL_Texture* background = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, 2);
    if (background == NULL) {
        printf("Failed to create background texture\n");
        printf("SDL2 Error: %d\n", SDL_GetError());
        return -1;
    }

    // Set the start and end colors (RGBA)
    Uint32 pixels[2] = {
        0x02CCFFFF, // Red
        0x72E2FFFF  // Blue
    };

    // Update the texture with the pixel data
    SDL_UpdateTexture(background, NULL, pixels, sizeof(Uint32));

    // Set the texture filtering mode to linear interpolation
    SDL_SetTextureBlendMode(background, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(background, SDL_ScaleModeLinear);
    
    playerParts[0] = loadTexture("assets/textures/player_backarm.png");
    playerParts[1] = loadTexture("assets/textures/player_backleg.png");
    playerParts[2] = loadTexture("assets/textures/player_body.png");
    playerParts[3] = loadTexture("assets/textures/player_frontleg.png");
    playerParts[4] = loadTexture("assets/textures/player_head.png");
    playerParts[5] = loadTexture("assets/textures/player_frontarm.png");
    hat = loadTexture("assets/textures/classic_hat.png");

    w = world_init(blocks);

    uint64_t start = SDL_GetPerformanceCounter();
    uint64_t next = SDL_GetPerformanceCounter();

    int i = 0;
    bool keep_window_open = true;
    while(keep_window_open)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0)
        {
            switch(e.type)
            {
                case SDL_QUIT:
                    keep_window_open = false;
                    break;
                case SDL_KEYUP:
                    if (e.key.keysym.sym == SDLK_F2) {
                        int width, height;
                        SDL_GetWindowSize(window, &width, &height);
                        SDL_Surface *sshot = SDL_CreateRGBSurface(0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
                        SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
                        IMG_SavePNG(sshot, "screenshot.png");
                        SDL_FreeSurface(sshot);
                    } else if (e.key.keysym.sym == SDLK_F3) {
                        SDL_Texture *sshot = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WORLD_WIDTH * 16, WORLD_HEIGHT * 16);
                        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, WORLD_WIDTH * 16, WORLD_HEIGHT * 16, 32, SDL_PIXELFORMAT_RGBA32);
                        
                        SDL_SetRenderTarget(renderer, sshot);
                        world_render_range(w, 0, WORLD_WIDTH, 0, WORLD_HEIGHT, WORLD_WIDTH / 2, WORLD_HEIGHT / 2, blocks, renderer);
                        SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch);
                        SDL_SetRenderTarget(renderer, NULL);
                        
                        IMG_SavePNG(surface, "world.png");

                        SDL_FreeSurface(surface);
                        SDL_DestroyTexture(sshot);
                    }
                    break;
            }
        }
        
        next = SDL_GetPerformanceCounter();
        double delta = (double)(next - start) / SDL_GetPerformanceFrequency();
        double fps = 1 / delta;

        if (fps < 144) {
            const uint8_t* keys = SDL_GetKeyboardState(NULL);

            float moveX = (keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * 5;
            float moveY = (keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]) * 5;

            playerframe += fabsf(moveX) * 8 * delta;
            if (playerframe >= 17) {
                playerframe -= 12;
            }

            if (moveX < 0) {
                playerside = 0;
                if (playerstate == idle)
                    playerframe = 5;
                playerstate = walking;
            } else if (moveX > 0) {
                playerside = 1;
                if (playerstate == idle)
                    playerframe = 5;
                playerstate = walking;
            } else if (moveX == 0) {
                playerframe = 0;
                playerstate = idle;
            }

            playerx += moveX * delta;
            playery += moveY * delta;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, background, NULL, NULL);

            render();
            SDL_RenderPresent(renderer);
            
            char title[64];
            snprintf(title, 64, "Survival Game -- %.2f fps", fps);
            SDL_SetWindowTitle(window, title);
            
            start = next;
        }
    }

    SDL_DestroyWindow(window);
    for (int i = 0; i < arrlen(blocks); i++) {
        SDL_DestroyTexture(blocks[i].value.tex);
        SDL_DestroyTexture(blocks[i].value.foliage);
    }
    shfree(blocks);
    SDL_DestroyTexture(background);
    for (int i = 0; i < 6; i++) {
        SDL_DestroyTexture(playerParts[i]);
    }
    SDL_DestroyTexture(hat);
    SDL_DestroyRenderer(renderer);

    free_assets(&assets);

    return 0;
}