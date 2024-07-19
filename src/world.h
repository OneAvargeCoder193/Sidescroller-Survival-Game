#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define WORLD_WIDTH 500
#define WORLD_HEIGHT 250

typedef enum blockshape {
    solid,
    edges,
    liquid
} blockshape;

typedef struct block {
    SDL_Texture* tex;
    SDL_Texture* foliage;
    bool transparent;
    blockshape shape;
} block;

block create_block(SDL_Texture* tex, SDL_Texture* foliage, bool transparent, blockshape shape);

typedef struct world {
    uint16_t blocks[WORLD_WIDTH][WORLD_HEIGHT];
} world;

world world_init(block* blocks);

uint8_t world_getblock(world w, int x, int y);
uint8_t world_getdata(world w, int x, int y);
uint16_t world_getblockdata(world w, int x, int y);

void world_setblock(world w, int x, int y, uint8_t v);
void world_setdata(world w, int x, int y, uint8_t v);
void world_setblockdata(world w, int x, int y, uint16_t v);

void world_render_range(world w, int minx, int maxx, int miny, int maxy, float camx, float camy, block* blocks, SDL_Renderer* renderer);
void world_render(world w, float camx, float camy, block* blocks, SDL_Renderer* renderer);

#endif