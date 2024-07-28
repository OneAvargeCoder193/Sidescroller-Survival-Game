#include "utils.h"

#include <stdio.h>

TTF_Font* font;

int mouseLeftReleased = 0;

uint32_t murmur_hash_combine(int32_t h1, int32_t h2) {
    const uint32_t seed = 0; // Initial seed
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const int32_t r1 = 15;
    const int32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    uint32_t k1 = h1;
    uint32_t k2 = h2;

    k1 *= c1;
    k1 = (k1 << r1) | (k1 >> (32 - r1));
    k1 *= c2;
    hash ^= k1;
    hash = (hash << r2) | (hash >> (32 - r2));
    hash = hash * m + n;

    k2 *= c1;
    k2 = (k2 << r1) | (k2 >> (32 - r1));
    k2 *= c2;
    hash ^= k2;
    hash = (hash << r2) | (hash >> (32 - r2));
    hash = hash * m + n;

    hash ^= 8; // Length of the data (two 4-byte integers)

    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;
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

SDL_Rect text_rect(int x, int y, char *text, int fontsize, TTF_Font *font, enum textalignx xalign, enum textaligny yalign) {
    TTF_SetFontSize(font, fontsize);
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, (SDL_Color){255, 255, 255, 255});

    SDL_Rect rect;
    rect.w = surface->w;
    rect.h = surface->h;
    
    switch (xalign) {
    case XALIGN_LEFT:
        rect.x = x;
        break;
    case XALIGN_RIGHT:
        rect.x = x - surface->w;
        break;
    case XALIGN_CENTER:
        rect.x = x - surface->w / 2;
        break;
    }

    switch (yalign) {
    case YALIGN_BOTTOM:
        rect.y = y;
        break;
    case YALIGN_TOP:
        rect.y = y - surface->h;
        break;
    case YALIGN_CENTER:
        rect.y = y - surface->h / 2;
        break;
    }

    SDL_FreeSurface(surface);
    return rect;
}

void render_text(SDL_Renderer *renderer, int x, int y, char *text, int fontsize, SDL_Color textColor, TTF_Font *font, enum textalignx xalign, enum textaligny yalign) {
    TTF_SetFontSize(font, fontsize);
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Surface* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect rect;
    rect.w = texture->w;
    rect.h = texture->h;
    
    switch (xalign) {
    case XALIGN_LEFT:
        rect.x = x;
        break;
    case XALIGN_RIGHT:
        rect.x = x - texture->w;
        break;
    case XALIGN_CENTER:
        rect.x = x - texture->w / 2;
        break;
    }

    switch (yalign) {
    case YALIGN_BOTTOM:
        rect.y = y;
        break;
    case YALIGN_TOP:
        rect.y = y - texture->h;
        break;
    case YALIGN_CENTER:
        rect.y = y - texture->h / 2;
        break;
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
}

int render_button(SDL_Renderer* renderer, int x, int y, const char* text, int fontsize, SDL_Color color, TTF_Font* font, enum textalignx xalign, enum textaligny yalign) {
    TTF_SetFontSize(font, fontsize);
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Surface* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect hoverRect;
    hoverRect.w = texture->w;
    hoverRect.h = texture->h;
    
    switch (xalign) {
    case XALIGN_LEFT:
        hoverRect.x = x;
        break;
    case XALIGN_RIGHT:
        hoverRect.x = x - texture->w;
        break;
    case XALIGN_CENTER:
        hoverRect.x = x - texture->w / 2;
        break;
    }

    switch (yalign) {
    case YALIGN_BOTTOM:
        hoverRect.y = y;
        break;
    case YALIGN_TOP:
        hoverRect.y = y - texture->h;
        break;
    case YALIGN_CENTER:
        hoverRect.y = y - texture->h / 2;
        break;
    }

    SDL_Point m;
    SDL_GetMouseState(&m.x, &m.y);

    int hovered = SDL_PointInRect(&m, &hoverRect);

    SDL_Rect rect;
    rect.w = texture->w;
    rect.h = texture->h;
    
    switch (xalign) {
    case XALIGN_LEFT:
        rect.x = x;
        break;
    case XALIGN_RIGHT:
        rect.x = x - texture->w;
        break;
    case XALIGN_CENTER:
        rect.x = x - texture->w / 2;
        break;
    }

    switch (yalign) {
    case YALIGN_BOTTOM:
        rect.y = y;
        break;
    case YALIGN_TOP:
        rect.y = y - texture->h;
        break;
    case YALIGN_CENTER:
        rect.y = y - texture->h / 2;
        break;
    }

    if (hovered) {
        float sizeDiff = (rect.w + 8) / (float)rect.w;
        rect.x += (-rect.w / 2 * sizeDiff) + rect.w / 2;
        rect.y += (-rect.h / 2 * sizeDiff) + rect.h / 2;
        rect.w *= sizeDiff;
        rect.h *= sizeDiff;
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);

    // printf("%d\n", hovered);
    // if (mouseLeftReleased) {
    //     printf("True\n");
    // }

    return hovered && mouseLeftReleased;
}