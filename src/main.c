#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <png.h>

#define FNL_IMPL

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "utils.h"
#include "world.h"
#include "assets.h"
#include "entity.h"

#include "state.h"

SDL_Renderer *renderer;

void render() {

    
}

SDL_Texture* loadTexture(const char* path) {
    return IMG_LoadTexture(renderer, path);
}

struct img {
    int x; int y; char* path;
};

void read_png_file(const char* filename, int* width, int* height, png_bytepp* row_pointers, png_byte* bit_depth, png_byte* color_type) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort();

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_png(png, info, PNG_TRANSFORM_IDENTITY, PNG_INTERLACE_NONE);

    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    *bit_depth = png_get_bit_depth(png, info);
    *color_type = png_get_color_type(png, info);

    *row_pointers = png_get_rows(png, info);

    fclose(fp);
}

// Function to write PNG file
void write_png_file(const char* filename, int width, int height, int bit_depth, int color_type, png_bytepp row_pointers) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s for writing\n", filename);
        exit(EXIT_FAILURE);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fprintf(stderr, "Error creating PNG write structure\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        fprintf(stderr, "Error creating PNG info structure\n");
        png_destroy_write_struct(&png, (png_infopp)NULL);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error during PNG write\n");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        width, height,
        bit_depth,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png, info);

    png_write_image(png, row_pointers);

    png_write_end(png, NULL);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

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
                        int numX = ceil((WORLD_WIDTH * 16) / 16384.0);
                        int numY = ceil((WORLD_HEIGHT * 16) / 16384.0);

                        mkdir("world_pic");

                        struct img* images = NULL;

                        for (int x = 0; x < numX; x++) {
                            for (int y = 0; y < numY; y++) {
                                int width = min(WORLD_WIDTH * 16 - x * 16384, 16383);
                                int height = min(WORLD_HEIGHT * 16 - y * 16384, 16383);
                                
                                SDL_Texture *sshot = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
                                SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
                                
                                SDL_SetRenderTarget(renderer, sshot);
                                world_render_range(&w, x * 1024, x * 1024 + width / 16, y * 1024, y * 1024 + height / 16, x * 1024 + width / 32, y * 1024 + height / 32, blocks, renderer);
                                SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch);
                                SDL_SetRenderTarget(renderer, NULL);

                                char* filename = malloc(256);
                                snprintf(filename, 256, "world_pic/world_%d_%d.png", x, y);
                                IMG_SavePNG(surface, filename);

                                arrput(images, ((struct img){x, y, filename}));

                                SDL_FreeSurface(surface);
                                SDL_DestroyTexture(sshot);
                            }
                        }

                        png_byte bit_depth;
                        png_byte color_type;
                        png_bytepp row_pointers;

                        png_byte** out = (png_byte**)malloc((WORLD_HEIGHT * 16) * sizeof(png_byte*));
                        for (int y = 0; y < WORLD_HEIGHT * 16; y++) {
                            out[y] = malloc(WORLD_WIDTH * 16 * 4);
                            memset(out[y], 255, WORLD_WIDTH * 16 * 4);
                        }

                        for (int i = 0; i < arrlen(images); i++) {
                            int width, height;
                            read_png_file(images[i].path, &width, &height, &row_pointers, &bit_depth, &color_type);
                            for (int y = 0; y < height; y++) {
                                memcpy(out[images[i].y * 16384 + y] + images[i].x * 16384 * 4, row_pointers[y], width * 4);
                            }
                        }

                        write_png_file("world.png", WORLD_WIDTH * 16, WORLD_HEIGHT * 16, bit_depth, color_type, out);

                        for (int y = 0; y < WORLD_HEIGHT * 16; ++y) {
                            free(out[y]);
                        }
                        free(out);

                        for (int i = 0; i < arrlen(images); i++) {
                            remove(images[i].path);
                            free(images[i].path);
                        }

                        rmdir("world_pic");

                        arrfree(images);
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