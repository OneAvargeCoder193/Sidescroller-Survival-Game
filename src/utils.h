#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "cJSON.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

extern TTF_Font* font;
extern int mouseLeftReleased;

enum textalignx {
    XALIGN_CENTER,
    XALIGN_LEFT,
    XALIGN_RIGHT
};

enum textaligny {
    YALIGN_CENTER,
    YALIGN_TOP,
    YALIGN_BOTTOM
};

uint32_t murmur_hash_combine(int32_t h1, int32_t h2);
const cJSON* load_json(const char *filepath);
SDL_Rect text_rect(int x, int y, char *text, int fontsize, TTF_Font *font, enum textalignx xalign, enum textaligny yalign);
void render_text(SDL_Renderer *renderer, int x, int y, char *text, int fontsize, SDL_Color textColor, TTF_Font *font, enum textalignx xalign, enum textaligny yalign);

int render_button(SDL_Renderer* renderer, int x, int y, const char* text, int fontsize, SDL_Color color, TTF_Font* font, enum textalignx xalign, enum textaligny yalign);

int render_nine_tile(SDL_Renderer* renderer, SDL_Rect* rect, SDL_Texture* tex);
int render_textured_button(SDL_Renderer* renderer, SDL_Rect* rect, const char* text, int fontsize, SDL_Color color, TTF_Font* font, SDL_Texture* idle, SDL_Texture* hovered, SDL_Texture* pressed);

typedef struct textinput {
    int maxLen;
    char* text;

    char* defaultText;
} textinput;

textinput create_textinput(int maxLen, char* defaultText);
void destroy_textinput(textinput inp);

void update_textinput(SDL_Event e, textinput* inp);
void render_textinput(SDL_Renderer* renderer, int x, int y, int w, textinput* inp, SDL_Texture* background);

#endif