#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#define FNL_IMPL

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "utils.h"
#include "world.h"
#include "assets.h"
#include "entity.h"

#include "state.h"

SDL_Renderer *renderer;

SDL_Texture* loadTexture(const char* path) {
    return IMG_LoadTexture(renderer, path);
}

Mix_Music* music = NULL;
Mix_Chunk* test = NULL;

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize the SDL2 library\n");
        printf("SDL2 Error: %d\n", SDL_GetError());
        return -1;
    }

    if (TTF_Init() < 0) {
        printf("Failed to initialize the TTF library\n");
        printf("TTF Error: %d\n", TTF_GetError());
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Failed to initialize the mixer library\n");
        printf("Mix Error: %d\n", Mix_GetError());
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

    font = TTF_OpenFont("assets/font/Font.ttf", 24);
    if (!font) {
        printf("Failed to load font\n");
        printf("TTF Error: %d\n", TTF_GetError());
        return -1;
    }

    test = Mix_LoadWAV("assets/sfx/test.wav");
    music = Mix_LoadMUS("assets/music/important.wav");
    
    state_init();

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

    entity e = createEntity("game:mossman", WORLD_WIDTH / 2 + 15, WORLD_HEIGHT / 2, 0, 0);
    arrput(entities, e);

    uint64_t start = SDL_GetPerformanceCounter();
    uint64_t next = SDL_GetPerformanceCounter();

    int i = 0;
    bool keep_window_open = true;
    while(keep_window_open)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e) > 0)
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
                        save_world_to_png(&w, renderer);
                    } else if (e.key.keysym.sym == SDLK_h) {
                        Mix_PlayChannel(-1, test, 0);
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        mouseLeftReleased = 1;
                    }
                    break;
            }
        }
        
        next = SDL_GetPerformanceCounter();
        double delta = (double)(next - start) / SDL_GetPerformanceFrequency();
        double fps = 1 / delta;

        if (Mix_PlayingMusic() == 0) {
            Mix_PlayMusic(music, -1);
            Mix_VolumeMusic(32);
        }

        if (fps < 144) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, background, NULL, NULL);
            
            state_update(renderer, delta);
            state_draw(renderer);
            SDL_RenderPresent(renderer);
            
            char title[64];
            snprintf(title, 64, "Survival Game -- %.2f fps", fps);
            SDL_SetWindowTitle(window, title);
            
            start = next;
            
            if (mouseLeftReleased)
                mouseLeftReleased = 0;
        }
    }

    SDL_DestroyWindow(window);
    for (int i = 0; i < arrlen(blocks); i++) {
        SDL_DestroyTexture(blocks[i].value.tex);
        SDL_DestroyTexture(blocks[i].value.foliage);
        free(blocks[i].value.connects);
    }
    shfree(blocks);
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);

    TTF_CloseFont(font);
    
    Mix_FreeMusic(music);
    Mix_FreeChunk(test);

    for (int i = 0; i < arrlen(entities); i++) {
        destroyEntity(entities[i]);
    }
    arrfree(entities);

    for (int i = 0; i < shlen(entitytypes); i++) {
        destroyEntityType(entitytypes[i].value);
    }
    shfree(entitytypes);

    free_assets(&assets);

    return 0;
}