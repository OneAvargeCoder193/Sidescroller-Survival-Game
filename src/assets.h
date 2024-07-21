#ifndef ASSETS_H
#define ASSETS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "cJSON.h"
#include "utils.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif

typedef struct {
    char *key;
    SDL_Texture *value;
} TextureAsset;

typedef struct {
    char *key;
    const cJSON *value;
} JSONAsset;

typedef struct {
    TextureAsset *blockTextures; // stb_ds hashmap
    TextureAsset *entityTextures; // stb_ds hashmap
    JSONAsset *blocks;       // stb_ds hashmap
    JSONAsset *entities;       // stb_ds hashmap
    SDL_Renderer *renderer; // SDL renderer for creating textures
} Assets;

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *filepath);
void load_assets_from_folder(Assets *assets, const char *folder_path, const char *prefix);
void load_all_assets(Assets *assets);
void init_assets(Assets *assets, SDL_Renderer *renderer);
void free_assets(Assets *assets);

#endif // ASSETS_H