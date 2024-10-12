#include "assets.h"

#include "world.h"
#include "block.h"
#include "entity.h"

#include "stb_ds.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Assets assets;

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *filepath) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, filepath);
    if (!texture) {
        fprintf(stderr, "Error loading texture %s: %s\n", filepath, SDL_GetError());
    }
    return texture;
}

void remove_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        *dot = '\0';
    }
}

void load_assets_from_folder_png(Assets *assets, const char *folder_path, const char *prefix, TextureAsset** out) {
#if defined(_WIN32) || defined(_WIN64)
    WIN32_FIND_DATA find_file_data;
    HANDLE h_find = INVALID_HANDLE_VALUE;
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*", folder_path);
    
    h_find = FindFirstFile(search_path, &find_file_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening directory %s\n", folder_path);
        return;
    }

    do {
        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s\\%s", folder_path, find_file_data.cFileName);

        const char* noExtName = strdup(find_file_data.cFileName);
        remove_extension(noExtName);

        char key[1024];
        snprintf(key, sizeof(key), "%s:%s", prefix, noExtName);

        free(noExtName);

        if (strstr(find_file_data.cFileName, ".png")) {
            SDL_Texture *texture = load_texture(assets->renderer, filepath);
            if (texture) {
                shput(*out, strdup(key), texture);
            }
        }
    } while (FindNextFile(h_find, &find_file_data) != 0);

    FindClose(h_find);
#else
    DIR *dir = opendir(folder_path);
    if (!dir) {
        fprintf(stderr, "Error opening directory %s\n", folder_path);
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", folder_path, entry->d_name);

        struct stat path_stat;
        if (stat(filepath, &path_stat) == -1) {
            perror("stat");
            continue;
        }
        if (S_ISDIR(path_stat.st_mode)) {
            continue;
        }

        
        const char* noExtName = strdup(entry->d_name);
        remove_extension(noExtName);

        char key[1024];
        snprintf(key, sizeof(key), "%s:%s", prefix, noExtName);

        free(noExtName);
        
        if (strstr(entry->d_name, ".png")) {
            SDL_Texture *texture = load_texture(assets->renderer, filepath);
            if (texture) {
                shput(*out, strdup(key), texture);
            }
        }
    }
    closedir(dir);
#endif
}

void load_assets_from_folder_json(Assets *assets, const char *folder_path, const char *prefix, JSONAsset** out) {
#if defined(_WIN32) || defined(_WIN64)
    WIN32_FIND_DATA find_file_data;
    HANDLE h_find = INVALID_HANDLE_VALUE;
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*", folder_path);
    
    h_find = FindFirstFile(search_path, &find_file_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening directory %s\n", folder_path);
        return;
    }

    do {
        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s\\%s", folder_path, find_file_data.cFileName);

        const char* noExtName = strdup(find_file_data.cFileName);
        remove_extension(noExtName);

        char key[1024];
        snprintf(key, sizeof(key), "%s:%s", prefix, noExtName);

        free(noExtName);

        if (strstr(find_file_data.cFileName, ".json")) {
            const cJSON *json = load_json(filepath);
            if (json) {
                shput(*out, strdup(key), json);
            }
        }
    } while (FindNextFile(h_find, &find_file_data) != 0);

    FindClose(h_find);
#else
    DIR *dir = opendir(folder_path);
    if (!dir) {
        fprintf(stderr, "Error opening directory %s\n", folder_path);
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", folder_path, entry->d_name);

        struct stat path_stat;
        if (stat(filepath, &path_stat) == -1) {
            perror("stat");
            continue;
        }
        if (S_ISDIR(path_stat.st_mode)) {
            continue;
        }

        
        const char* noExtName = strdup(entry->d_name);
        remove_extension(noExtName);

        char key[1024];
        snprintf(key, sizeof(key), "%s:%s", prefix, noExtName);

        free(noExtName);
        
        if (strstr(entry->d_name, ".json")) {
            const cJSON *json = load_json(filepath);
            if (json) {
                shput(*out, strdup(key), json);
            }
        }
    }
    closedir(dir);
#endif
}

void load_all_assets(Assets *assets) {
    load_assets_from_folder_json(assets, "assets/blocks", "game", &assets->blocks);
    load_assets_from_folder_json(assets, "assets/entities", "game", &assets->entities);
    load_assets_from_folder_png(assets, "assets/textures", "game", &assets->textures);
    load_assets_from_folder_png(assets, "assets/ui", "game", &assets->uiTextures);
    load_assets_from_folder_png(assets, "assets/blocks/textures", "game", &assets->blockTextures);
    load_assets_from_folder_png(assets, "assets/entities/textures", "game", &assets->entityTextures);
    load_assets_from_folder_png(assets, "assets/particles/textures", "game", &assets->particleTextures);
}

void init_assets(Assets *assets, SDL_Renderer *renderer) {
    assets->textures = NULL;
    assets->uiTextures = NULL;
    assets->blockTextures = NULL;
    assets->entityTextures = NULL;
    assets->particleTextures = NULL;
    assets->blocks = NULL;
    assets->entities = NULL;
    assets->renderer = renderer;
    
    load_all_assets(assets);
    
    block air = create_block(NULL, NULL, true, shape_solid);
    shput(blocks, "game:air", air);
    // Initialize blocks
    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        shput(blocks, assets->blocks[i].key, air);
    }

    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        registerBlock(assets->blocks[i].key, assets->blocks[i].value, assets);
    }

    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        registerConnects(assets->blocks[i].key, assets->blocks[i].value);
    }

    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        fixConnectConflict(assets->blocks[i].key);
    }

    for (size_t i = 0; i < shlen(assets->entities); ++i) {
        registerEntityType(assets->entities[i].key, assets->entities[i].value, assets);
    }
}

void free_assets(Assets *assets) {
    for (size_t i = 0; i < shlen(assets->textures); ++i) {
        SDL_DestroyTexture(assets->textures[i].value);
        free(assets->textures[i].key);
    }
    shfree(assets->textures);

    for (size_t i = 0; i < shlen(assets->uiTextures); ++i) {
        SDL_DestroyTexture(assets->uiTextures[i].value);
        free(assets->uiTextures[i].key);
    }
    shfree(assets->uiTextures);

    for (size_t i = 0; i < shlen(assets->blockTextures); ++i) {
        SDL_DestroyTexture(assets->blockTextures[i].value);
        free(assets->blockTextures[i].key);
    }
    shfree(assets->blockTextures);

    for (size_t i = 0; i < shlen(assets->entityTextures); ++i) {
        SDL_DestroyTexture(assets->entityTextures[i].value);
        free(assets->entityTextures[i].key);
    }
    shfree(assets->entityTextures);

    for (size_t i = 0; i < shlen(assets->particleTextures); ++i) {
        SDL_DestroyTexture(assets->particleTextures[i].value);
        free(assets->particleTextures[i].key);
    }
    shfree(assets->particleTextures);
    
    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        cJSON_free((cJSON *)assets->blocks[i].value);
        free(assets->blocks[i].key);
    }
    shfree(assets->blocks);

    for (size_t i = 0; i < shlen(assets->entities); ++i) {
        cJSON_free((cJSON *)assets->entities[i].value);
        free(assets->entities[i].key);
    }
    shfree(assets->entities);
}