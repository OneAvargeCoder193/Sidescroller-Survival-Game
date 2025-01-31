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
    genLight,
    genData,
    genDone
};

#define WORLD_LAYER 0
#define WALL_LAYER 1

extern uint32_t world_blocks[WORLD_WIDTH][WORLD_HEIGHT][2];
extern float world_heightmap[WORLD_WIDTH];

typedef struct world {
    uint32_t (*blocks)[WORLD_HEIGHT][2];
    uint8_t (*light)[WORLD_HEIGHT];
    char path[260];
    float *heightMap;
    int genIdx;
    int genMax;
    bool finishedGen;
    bool finishedGenData;
    int* updateLightingPos;
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

void world_updatedata(world* w, int x, int y, int layer);
void world_gendatarange(world* w, int minx, int miny, int maxx, int maxy);

void world_genlight(world* w);

void world_drawBlockPos(SDL_Renderer* renderer, struct blockhash* blocks, int b, uint8_t light, int bx, int by, int x, int y);
void world_drawBlockFoliagePos(SDL_Renderer* renderer, struct blockhash* blocks, int b, uint8_t light, int bx, int by, int x, int y);

uint8_t world_getblock(world* w, int x, int y);
uint32_t world_getblockdata(world* w, int x, int y);
uint32_t world_getblockanddata(world* w, int x, int y);

uint8_t world_getwall(world* w, int x, int y);
uint32_t world_getwalldata(world* w, int x, int y);
uint32_t world_getwallanddata(world* w, int x, int y);

uint8_t world_getblocklayer(world* w, int x, int y, int layer);
uint32_t world_getblockdatalayer(world* w, int x, int y, int layer);
uint32_t world_getblockanddatalayer(world* w, int x, int y, int layer);

void world_setblock(world *w, int x, int y, uint8_t v);
void world_setblockdata(world *w, int x, int y, uint32_t v);
void world_setblockanddata(world *w, int x, int y, uint32_t v);

void world_setwall(world *w, int x, int y, uint8_t v);
void world_setwalldata(world *w, int x, int y, uint32_t v);
void world_setwallanddata(world *w, int x, int y, uint32_t v);

void world_setblocklayer(world *w, int x, int y, int layer, uint8_t v);
void world_setblockdatalayer(world *w, int x, int y, int layer, uint32_t v);
void world_setblockanddatalayer(world *w, int x, int y, int layer, uint32_t v);

void world_render_range(world* w, int minx, int maxx, int miny, int maxy, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer);
void world_render(world* w, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer);

void world_save(world* w, FILE* out);
void world_load(world* w, FILE* in);

void save_world_to_png(world* w, SDL_Renderer* renderer, char* out);

#endif