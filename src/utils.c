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
        rect.y = y - surface->h;
        break;
    case YALIGN_TOP:
        rect.y = y;
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
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

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
        rect.y = y - surface->h;
        break;
    case YALIGN_TOP:
        rect.y = y;
        break;
    case YALIGN_CENTER:
        rect.y = y - surface->h / 2;
        break;
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
}

int render_button(SDL_Renderer* renderer, int x, int y, const char* text, int fontsize, SDL_Color color, TTF_Font* font, enum textalignx xalign, enum textaligny yalign) {
    TTF_SetFontSize(font, fontsize);
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect hoverRect;
    hoverRect.w = surface->w;
    hoverRect.h = surface->h;
    
    switch (xalign) {
    case XALIGN_LEFT:
        hoverRect.x = x;
        break;
    case XALIGN_RIGHT:
        hoverRect.x = x - surface->w;
        break;
    case XALIGN_CENTER:
        hoverRect.x = x - surface->w / 2;
        break;
    }

    switch (yalign) {
    case YALIGN_BOTTOM:
        hoverRect.y = y - surface->h;
        break;
    case YALIGN_TOP:
        hoverRect.y = y;
        break;
    case YALIGN_CENTER:
        hoverRect.y = y - surface->h / 2;
        break;
    }

    SDL_Point m;
    SDL_GetMouseState(&m.x, &m.y);

    int hovered = SDL_PointInRect(&m, &hoverRect);

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
        rect.y = y - surface->h;
        break;
    case YALIGN_TOP:
        rect.y = y;
        break;
    case YALIGN_CENTER:
        rect.y = y - surface->h / 2;
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

    return hovered && mouseLeftReleased;
}

int render_nine_tile(SDL_Renderer* renderer, SDL_Rect* rect, SDL_Texture* tex) {
    for (int ry = rect->y; ry < rect->y + rect->h; ry += 16) {
        int h = 16;
        if (ry + 16 >= rect->y + rect->h) {
            h = rect->y + rect->h - ry;
        }

        for (int rx = rect->x; rx < rect->x + rect->w; rx += 16) {
            int w = 16;
            if (rx + 16 >= rect->x + rect->w) {
                w = rect->x + rect->w - rx;
            }
            
            SDL_Rect src;
            src.x = 8;
            src.y = 8;
            src.w = w / 2;
            src.h = h / 2;
            
            SDL_Rect dst;
            dst.x = rx;
            dst.y = ry;
            dst.w = w;
            dst.h = h;
            SDL_RenderCopy(renderer, tex, &src, &dst);
        }
    }

    for (int rx = rect->x; rx < rect->x + rect->w; rx += 16) {
        int w = 16;
        if (rx + 16 >= rect->x + rect->w) {
            w = rect->x + rect->w - rx;
        }

        SDL_Rect src;
        src.x = 8;
        src.y = 0;
        src.w = w / 2;
        src.h = 8;
        
        SDL_Rect dst;
        dst.x = rx;
        dst.y = rect->y - 16;
        dst.w = w;
        dst.h = 16;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    for (int rx = rect->x; rx < rect->x + rect->w; rx += 16) {
        int w = 16;
        if (rx + 16 >= rect->x + rect->w) {
            w = rect->x + rect->w - rx;
        }

        SDL_Rect src;
        src.x = 8;
        src.y = 16;
        src.w = w / 2;
        src.h = 8;
        
        SDL_Rect dst;
        dst.x = rx;
        dst.y = rect->y + rect->h;
        dst.w = w;
        dst.h = 16;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    for (int ry = rect->y; ry < rect->y + rect->h; ry += 16) {
        int h = 16;
        if (ry + 16 >= rect->y + rect->h) {
            h = rect->y + rect->h - ry;
        }

        SDL_Rect src;
        src.x = 0;
        src.y = 8;
        src.w = 8;
        src.h = h / 2;
        
        SDL_Rect dst;
        dst.x = rect->x - 16;
        dst.y = ry;
        dst.w = 16;
        dst.h = h;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    for (int ry = rect->y; ry < rect->y + rect->h; ry += 16) {
        int h = 16;
        if (ry + 16 >= rect->y + rect->h) {
            h = rect->y + rect->h - ry;
        }

        SDL_Rect src;
        src.x = 16;
        src.y = 8;
        src.w = 8;
        src.h = h / 2;
        
        SDL_Rect dst;
        dst.x = rect->x + rect->w;
        dst.y = ry;
        dst.w = 16;
        dst.h = h;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    {
        SDL_Rect src;
        src.x = 0;
        src.y = 0;
        src.w = 8;
        src.h = 8;
        
        SDL_Rect dst;
        dst.x = rect->x - 16;
        dst.y = rect->y - 16;
        dst.w = 16;
        dst.h = 16;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    {
        SDL_Rect src;
        src.x = 16;
        src.y = 0;
        src.w = 8;
        src.h = 8;
        
        SDL_Rect dst;
        dst.x = rect->x + rect->w;
        dst.y = rect->y - 16;
        dst.w = 16;
        dst.h = 16;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    {
        SDL_Rect src;
        src.x = 16;
        src.y = 16;
        src.w = 8;
        src.h = 8;
        
        SDL_Rect dst;
        dst.x = rect->x + rect->w;
        dst.y = rect->y + rect->h;
        dst.w = 16;
        dst.h = 16;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }

    {
        SDL_Rect src;
        src.x = 0;
        src.y = 16;
        src.w = 8;
        src.h = 8;
        
        SDL_Rect dst;
        dst.x = rect->x - 16;
        dst.y = rect->y + rect->h;
        dst.w = 16;
        dst.h = 16;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }
}

int render_textured_button(SDL_Renderer* renderer, SDL_Rect* rect, const char* text, int fontsize, SDL_Color color, TTF_Font* font, SDL_Texture* idle, SDL_Texture* hovered, SDL_Texture* pressed) {
    SDL_Point m;
    int left = SDL_GetMouseState(&m.x, &m.y) & SDL_BUTTON(SDL_BUTTON_LEFT);

    int hov = SDL_PointInRect(&m, rect);

    SDL_Texture* t = idle;
    if (hov)
        t = hovered;
    if (hov && left)
        t = pressed;
    
    render_nine_tile(renderer, rect, t);

    TTF_SetFontSize(font, fontsize);
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect r;
    r.x = rect->x + rect->w / 2 - surface->w / 2;
    r.y = rect->y + rect->h / 2 - surface->h / 2;
    r.w = surface->w;
    r.h = surface->h;
    
    SDL_RenderCopy(renderer, texture, NULL, &r);

    SDL_DestroyTexture(texture);
    
    return hov && mouseLeftReleased;
}

textinput create_textinput(int maxLen, char* defaultText) {
    textinput inp;
    inp.maxLen = maxLen;
    inp.text = malloc(maxLen + 1);
    memset(inp.text, 0, maxLen + 1);
    inp.defaultText = strdup(defaultText);
    return inp;
}

void destroy_textinput(textinput inp) {
    free(inp.text);
    free(inp.defaultText);
}

void update_textinput(SDL_Event e, textinput* inp) {
    switch (e.type) {
    case SDL_KEYDOWN: {
        if (e.key.keysym.sym == SDLK_BACKSPACE) {
            if (strlen(inp->text) > 0) {
                inp->text[strlen(inp->text) - 1] = '\0';
            }
        }
        break;
    }
    case SDL_TEXTINPUT: {
        if( !( SDL_GetModState() & KMOD_CTRL && ( e.text.text[ 0 ] == 'c' || e.text.text[ 0 ] == 'C' || e.text.text[ 0 ] == 'v' || e.text.text[ 0 ] == 'V' ) ) )
        {
            int len = strlen(e.text.text);
            if (strlen(inp->text) + len >= inp->maxLen)
                len = inp->maxLen - strlen(inp->text);
            strncat(inp->text, e.text.text, len);
        }
    }
    }
}

void render_textinput(SDL_Renderer* renderer, int x, int y, int w, textinput* inp, SDL_Texture* background) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = 36;
    render_nine_tile(renderer, &rect, background);

    if (strlen(inp->text) != 0)
        render_text(renderer, x, y, inp->text, 48, (SDL_Color){0, 0, 0, 255}, font, XALIGN_LEFT, YALIGN_TOP);
    else
        render_text(renderer, x, y, inp->defaultText, 48, (SDL_Color){127, 127, 127, 255}, font, XALIGN_LEFT, YALIGN_TOP);
}