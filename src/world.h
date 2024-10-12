#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "FastNoiseLite.h"

#include "assets.h"
#include "cJSON.h"

#include "block.h"

#define WORLD_WIDTH 2000
#define WORLD_HEIGHT 500

struct ffp {
    int x; int y; int block;
};

enum worldstate {
    genEmpty,
    genBlocks,
    genPlaceLava,
    genLiquids,
    genVegetation,
    genData,
    genDone
};

extern uint32_t world_blocks[WORLD_WIDTH][WORLD_HEIGHT];
extern float world_heightmap[WORLD_WIDTH];

typedef struct world {
    uint32_t (*blocks)[WORLD_HEIGHT];
    char path[260];
    float *heightMap;
    int genIdx;
    int genMax;
    bool finishedGen;
    bool finishedGenData;
    struct ffp* fillWaterPos;
    struct ffp* waterpos;
    fnl_state n;
    fnl_state cave;
    fnl_state rock;
    enum worldstate generateState;
} world;

extern world w;

world world_init(struct blockhash* blocks);

void world_genblock(world* w);
void world_gendata(world* w);
void world_placelava(world* w);
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

void world_save(world* w, FILE* out);
void world_load(world* w, FILE* in);

void save_world_to_png(world* w, SDL_Renderer* renderer, char* out);

#endif