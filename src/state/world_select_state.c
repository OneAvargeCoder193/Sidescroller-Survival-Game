#include "world_select_state.h"
#include <stdio.h>

#include "utils.h"

#include "state.h"
#include "world.h"
#include "assets.h"

#include "stb_ds.h"

SDL_Texture* button_idle;
SDL_Texture* button_hovered;
SDL_Texture* button_pressed;

textinput textInput;

char** worldPaths;

char *removeExt(char* myStr) {
    char *retStr;
    char *lastExt;
    if (myStr == NULL) return NULL;
    if ((retStr = malloc (strlen (myStr) + 1)) == NULL) return NULL;
    strcpy (retStr, myStr);
    lastExt = strrchr (retStr, '.');
    if (lastExt != NULL)
        *lastExt = '\0';
    return retStr;
}

char** get_worlds(const char *folder_path) {
    char** out = NULL;
#if defined(_WIN32) || defined(_WIN64)
    WIN32_FIND_DATA find_file_data;
    HANDLE h_find = INVALID_HANDLE_VALUE;
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*", folder_path);
    
    h_find = FindFirstFile(search_path, &find_file_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening directory %s\n", folder_path);
        return out;
    }

    do {
        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s\\%s", folder_path, find_file_data.cFileName);

        if (strstr(find_file_data.cFileName, ".bin")) {
            arrput(out, strdup(find_file_data.cFileName));
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

        if (strstr(entry->d_name, ".bin")) {
            arrput(out, strdup(entry->d_name));
        }
    }
    closedir(dir);
#endif
    return out;
}

void world_select_state_init(void) {
    button_idle = shget(assets.uiTextures, "game:text_button");
    button_hovered = shget(assets.uiTextures, "game:text_button_hovered");
    button_pressed = shget(assets.uiTextures, "game:text_button_pressed");

    worldPaths = get_worlds("worlds");

    textInput = create_textinput(10, "World Name");
}

void world_select_state_cleanup(void) {
    for (int i = 0; i < arrlen(worldPaths); i++) {
        free(worldPaths[i]);
    }
    arrfree(worldPaths);
    destroy_textinput(textInput);
}

void world_select_state_handle_events(SDL_Event e) {
    update_textinput(e, &textInput);
}

void world_select_state_update(SDL_Renderer* renderer, float delta) {

}

void world_select_state_draw(SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    render_text(renderer, width / 2, 24, "-Game Title-", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_CENTER, YALIGN_BOTTOM);
    
    if (render_button(renderer, 24, height - 152 - 24, "Singleplayer", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_LEFT, YALIGN_CENTER)) {
        state_set(STATE_LOADING);
    }
    
    SDL_Rect worldSelectR;
    worldSelectR.x = width / 3;
    worldSelectR.y = 0;
    worldSelectR.w = width / 3;
    worldSelectR.h = height;
    render_nine_tile(renderer, &worldSelectR, button_idle);

    for (int i = 0; i < arrlen(worldPaths); i++) {
        char* name = removeExt(worldPaths[i]);

        int x = width / 2 - 160;
        int y = 127 * i + 32;

        SDL_Rect worldR;
        worldR.x = x;
        worldR.y = y;
        worldR.w = 320;
        worldR.h = 95;

        render_nine_tile(renderer, &worldR, button_idle);
        
        SDL_Rect borderR;
        borderR.x = x + 13;
        borderR.y = y + 13;
        borderR.w = 69;
        borderR.h = 69;

        SDL_Rect imageR;
        imageR.x = x + 15;
        imageR.y = y + 15;
        imageR.w = 65;
        imageR.h = 65;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &borderR);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &imageR);

        render_text(renderer, x + 85, y + 13, name, 48, (SDL_Color){0, 0, 0, 255}, font, XALIGN_LEFT, YALIGN_TOP);

        SDL_Rect playR;
        playR.x = x + 217;
        playR.y = y + 52;
        playR.w = 90;
        playR.h = 30;

        if (render_textured_button(renderer, &playR, "Play", 48, (SDL_Color){0, 0, 0, 255}, font, button_idle, button_hovered, button_pressed)) {
            char path[260];
            sprintf(path, "worlds/%s", worldPaths[i]);
            FILE* fp = fopen(path, "rb");

            w = world_init(blocks);
            memcpy(w.path, path, strlen(path) + 1);
            world_load(&w, fp);

            fclose(fp);

            state_set(STATE_GAME);
        }
        
        SDL_Rect xR;
        xR.x = x + 287;
        xR.y = y;
        xR.w = 30;
        xR.h = 30;

        render_textured_button(renderer, &xR, "X", 48, (SDL_Color){255, 0, 0, 255}, font, NULL, NULL, NULL);

        free(name);
    }

    render_textinput(renderer, width / 2 - 160, height - 48, 320, &textInput, button_idle);

    render_button(renderer, 24, height - 88 - 24, "Multiplayer", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_LEFT, YALIGN_CENTER);
    render_button(renderer, 24, height - 24 - 24, "Settings", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_LEFT, YALIGN_CENTER);
}
