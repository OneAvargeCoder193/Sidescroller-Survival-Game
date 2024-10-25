#ifndef BLOCK_H
#define BLOCK_H

#include <SDL2/SDL.h>
#include "assets.h"
#include "cJSON.h"

#include <stdbool.h>

typedef enum blockshape {
    shape_solid,
    shape_edges,
    shape_log,
    shape_liquid
} blockshape;

typedef struct block {
    SDL_Texture* tex;
    SDL_Texture* foliage;
    SDL_Color* colors;
    int* connects;
    int layer;
    blockshape shape;
    bool collision;
} block;

struct blockhash {
    const char* key;
    block value;
};

extern struct blockhash* blocks;

block create_block(SDL_Texture* tex, SDL_Texture* foliage, bool transparent, blockshape shape);
void registerBlock(const char* key, const cJSON* json, Assets *assets);
void registerConnects(const char* key, const cJSON* json);
void fixConnectConflict(const char* key);

#endif