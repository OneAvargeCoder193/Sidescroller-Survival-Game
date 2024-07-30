#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "FastNoiseLite.h"

#include "assets.h"
#include "cJSON.h"

#define WORLD_WIDTH 500
#define WORLD_HEIGHT 250

typedef enum blockshape {
    shape_solid,
    shape_edges,
    shape_log,
    shape_liquid
} blockshape;

typedef struct block {
    SDL_Texture* tex;
    SDL_Texture* foliage;
    int* connects;
    blockshape shape;
    bool innerBorders;
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

struct ffp {
    int x; int y; int block;
};

enum worldstate {
    genEmpty,
    genBlocks,
    genLiquids,
    genVegetation,
    genData,
    genDone
};

typedef struct world {
    uint32_t blocks[WORLD_WIDTH][WORLD_HEIGHT];
    float heightMap[WORLD_WIDTH];
    int genIdx;
    bool finishedGen;
    bool finishedGenData;
    struct ffp* fillWaterPos;
    struct ffp* waterpos;
    fnl_state n;
    fnl_state rock;
    enum worldstate generateState;
} world;

extern world w;

world world_init(struct blockhash* blocks);

void world_genblock(world* w);
void world_gendata(world* w);
void world_fillliquids(world* w);
void world_addvegetation(world* w);

void world_updatedata(world* w, int x, int y);
void world_gendatarange(world* w, int minx, int miny, int maxx, int maxy);

uint8_t world_getblock(world* w, int x, int y);
uint32_t world_getdata(world* w, int x, int y);
uint32_t world_getblockdata(world* w, int x, int y);

void world_setblock(world *w, int x, int y, uint8_t v);
void world_setdata(world *w, int x, int y, uint32_t v);
void world_setblockdata(world *w, int x, int y, uint32_t v);

void world_render_range(world* w, int minx, int maxx, int miny, int maxy, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer);
void world_render(world* w, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer);

#endif