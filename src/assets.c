#include "assets.h"

#include "world.h"

#include "stb_ds.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *filepath) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, filepath);
    if (!texture) {
        fprintf(stderr, "Error loading texture %s: %s\n", filepath, SDL_GetError());
    }
    return texture;
}

const cJSON* load_json(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error opening JSON file %s\n", filepath);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    
    fclose(file);
    
    const cJSON *json = cJSON_Parse(data);
    
    free(data);
    
    if (!json) {
        fprintf(stderr, "Error parsing JSON file %s\n", filepath);
    }
    return json;
}

void remove_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        *dot = '\0';
    }
}

void load_assets_from_folder(Assets *assets, const char *folder_path, const char *prefix) {
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
                shput(assets->textures, strdup(key), texture);
            }
        } else if (strstr(find_file_data.cFileName, ".json")) {
            const cJSON *json = load_json(filepath);
            if (json) {
                shput(assets->blocks, strdup(key), json);
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
                shput(assets->textures, strdup(key), texture);
            }
        } else if (strstr(entry->d_name, ".json")) {
            const cJSON *json = load_json(filepath);
            if (json) {
                shput(assets->blocks, strdup(key), json);
            }
        }
    }
    closedir(dir);
#endif
}

void load_all_assets(Assets *assets) {
    load_assets_from_folder(assets, "assets/blocks", "game");
    load_assets_from_folder(assets, "assets/textures", "game");
    load_assets_from_folder(assets, "assets/blocks/textures", "game");
}

void init_assets(Assets *assets, SDL_Renderer *renderer) {
    assets->textures = NULL;
    assets->blocks = NULL;
    assets->renderer = renderer;
    
    load_all_assets(assets);
    
    shput(blocks, "game:air", create_block(NULL, NULL, true, solid));
    // Initialize blocks
    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        registerBlock(assets->blocks[i].key, assets->blocks[i].value, assets);
    }
}

void free_assets(Assets *assets) {
    for (size_t i = 0; i < shlen(assets->textures); ++i) {
        SDL_DestroyTexture(assets->textures[i].value);
        free(assets->textures[i].key);
    }
    shfree(assets->textures);
    
    for (size_t i = 0; i < shlen(assets->blocks); ++i) {
        cJSON_free((cJSON *)assets->blocks[i].value);
        free(assets->blocks[i].key);
    }
    shfree(assets->blocks);
}