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

world w;
SDL_Renderer *renderer;
block *blocks = NULL;

float camx = 0;
float camy = 0;

void render() {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    world_render(w, camx, camy, blocks, renderer);
}

SDL_Texture* loadTexture(const char* path, bool calculateVariants) {
    // if (!calculateVariants) {
        return IMG_LoadTexture(renderer, path);
    // }
    // SDL_Surface* surf = IMG_Load(path);

    // SDL_Surface* surfVars = calculateAllVariants(surf);
    // SDL_FreeSurface(surf);
    
    // SDL_Texture* res = SDL_CreateTextureFromSurface(renderer, surfVars);
    // SDL_FreeSurface(surfVars);
    // return res;
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
    
    arrput(blocks, create_block(NULL, NULL, true, false));
    arrput(blocks, create_block(loadTexture("res/textures/grass.png", true), loadTexture("res/textures/grass_foliage.png", true), false, true));
    arrput(blocks, create_block(loadTexture("res/textures/dirt.png", true), NULL, false, true));
    arrput(blocks, create_block(loadTexture("res/textures/water.png", false), NULL, true, false));
    arrput(blocks, create_block(loadTexture("res/textures/stone.png", true), NULL, false, true));

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
                    }
                    break;
            }
        }
        
        next = SDL_GetPerformanceCounter();
        double delta = (double)(next - start) / SDL_GetPerformanceFrequency();
        double fps = 1 / delta;

        if (fps < 144) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, background, NULL, NULL);

            render();
            SDL_RenderPresent(renderer);

            camx += delta;
            
            char title[64];
            snprintf(title, 64, "Survival Game -- %.2f fps", fps);
            SDL_SetWindowTitle(window, title);
            
            start = next;
        }
    }

    SDL_DestroyWindow(window);
    for (int i = 0; i < arrlen(blocks); i++) {
        SDL_DestroyTexture(blocks[i].tex);
        SDL_DestroyTexture(blocks[i].foliage);
    }
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);

    return 0;
}