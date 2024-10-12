#include "world.h"

#include "utils.h"

#include "stb_ds.h"

uint32_t world_blocks[WORLD_WIDTH][WORLD_HEIGHT];
float world_heightmap[WORLD_WIDTH];

world w = {NULL, {0}, NULL, 0, 0, 0, NULL, NULL, {0}, {0}, genEmpty};

int connectBlock(int a, int b) {
    return blocks[a].value.connects[b];
}

bool contains(struct ffp* p, int x, int y, int block) {
    for (int i = 0; i < arrlen(p); i++) {
        if (p[i].x == x && p[i].y == y && p[i].block == block) {
            return 1;
        }
    }
    return 0;
}

struct ffp* floodfill(world* w, int x, int y, int block) {
    struct ffp* points = NULL;
    struct ffp* allpoints = NULL;

    arrput(points, ((struct ffp){x, y, block}));
    arrput(allpoints, ((struct ffp){x, y, block}));

    while (arrlen(points) != 0) {
        struct ffp p = arrpop(points);

        if ((world_getblock(w, p.x + 1, p.y) == 0 || world_getblock(w, p.x + 1, p.y) == block) && !contains(allpoints, p.x + 1, p.y, p.block)) {
            arrput(points, ((struct ffp){p.x + 1, p.y, p.block}));
            arrput(allpoints, ((struct ffp){p.x + 1, p.y, p.block}));
        }

        if ((world_getblock(w, p.x - 1, p.y) == 0 || world_getblock(w, p.x - 1, p.y) == block) && !contains(allpoints, p.x - 1, p.y, p.block)) {
            arrput(points, ((struct ffp){p.x - 1, p.y, p.block}));
            arrput(allpoints, ((struct ffp){p.x - 1, p.y, p.block}));
        }

        if ((world_getblock(w, p.x, p.y - 1) == 0 || world_getblock(w, p.x, p.y - 1) == block) && !contains(allpoints, p.x, p.y - 1, p.block)) {
            arrput(points, ((struct ffp){p.x, p.y - 1, p.block}));
            arrput(allpoints, ((struct ffp){p.x, p.y - 1, p.block}));
        }
    }

    arrfree(points);

    return allpoints;
}

world world_init(struct blockhash* blocks) {
    world w = {0};
    w.blocks = world_blocks;
    w.heightMap = world_heightmap;
    w.genIdx = 0;
    w.genMax = WORLD_WIDTH * WORLD_HEIGHT;
    w.finishedGen = false;
    w.finishedGenData = false;
    w.fillWaterPos = NULL;
    w.waterpos = NULL;
    w.generateState = genBlocks;

    w.n = fnlCreateState();
    w.n.fractal_type = FNL_FRACTAL_FBM;
    w.n.octaves = 16;

    w.cave = fnlCreateState();
    w.cave.fractal_type = FNL_FRACTAL_FBM;
    w.cave.octaves = 16;
    
    w.rock = fnlCreateState();
    w.rock.noise_type = FNL_NOISE_CELLULAR;
    w.rock.cellular_distance_func = FNL_CELLULAR_DISTANCE_EUCLIDEAN;

    // for (int x = 0; x < WORLD_WIDTH; x++) {
    //     float off = -expf(-x / 300.0) * 300;
    //     off -= expf(-(WORLD_WIDTH - x) / 300.0) * 300;
    //     float scale = 1 - expf(-x / 300.0);
    //     scale *= 1 - expf(-(WORLD_WIDTH - x) / 300.0);
    //     w.heightMap[x] = fnlGetNoise2D(&w.n, x / 2.0, 0) * scale * 30 + off + WORLD_HEIGHT / 2 + 10;
    // }
    
    for (int x = 0; x < WORLD_WIDTH; x++) {
        float off = -expf(-x / 50.0) * 50;
        off -= expf(-(WORLD_WIDTH - x) / 50.0) * 50;
        float scale = 1 - expf(-x / 50.0);
        scale *= 1 - expf(-(WORLD_WIDTH - x) / 50.0);
        w.heightMap[x] = fnlGetNoise2D(&w.n, x / 2.0, 0) * scale * 30 + off + WORLD_HEIGHT / 2 + 10;
    }

    return w;
}

void world_genblock(world* w) {
    int i = w->genIdx;

    w->genIdx++;

    if (i == w->genMax - 1) {
        w->finishedGen = true;
        w->genIdx = 0;
        w->genMax = WORLD_WIDTH * WORLD_HEIGHT;
        w->generateState = genPlaceLava;
        return;
    }

    int x = i % WORLD_WIDTH;
    int y = i / WORLD_WIDTH;

    // double start = SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();

    float height = w->heightMap[x];
    const char* block = "game:air";
    if (y < height) {
        if (y >= WORLD_HEIGHT / 2) {
            block = "game:grass";
        } else {
            block = "game:dirt";
        }
    }
    if (y < height - 3) {
        block = "game:dirt";
    }
    // if (y < height - (fnlGetNoise2D(&w->n, x, y) / 2 + 0.5) * 20 + 30) {
    //     block = "game:stone";
    // }
    if (strcmp(block, "game:air") == 0 && y == WORLD_HEIGHT / 2) {
        arrput(w->fillWaterPos, ((struct ffp){x, y, shgeti(blocks, "game:water")}));
    }
    
    int caveMask = fnlGetNoise2D(&w->n, x - 2939, y + 2929) < -0.2;
    int thickCave = fabsf(fnlGetNoise2D(&w->n, x, y)) * fabsf(fnlGetNoise2D(&w->n, x + 5399, y + 3494)) < 0.04;
    int thinCave = fabsf(fnlGetNoise2D(&w->n, x, y)) < 0.1 && fabsf(fnlGetNoise2D(&w->n, x + 5399, y + 3494)) < 0.1;
    if (caveMask?thickCave:thinCave) {
        block = "game:air";
    }
    
    if (strcmp(block, "game:dirt") == 0) {
        int nx = x + fnlGetNoise2D(&w->n, x - 2283, y - 9872) * 50;
        int ny = y + fnlGetNoise2D(&w->n, x + 3939, y + 2939) * 50;
        if (fnlGetNoise2D(&w->rock, nx, ny) < -0.5 && y < height - 7) {
            block = "game:stone";
        }
    }

    if (strcmp(block, "game:stone") == 0) {
        int nx = x + fnlGetNoise2D(&w->n, x - 2283, y - 9872) * 10;
        int ny = y + fnlGetNoise2D(&w->n, x + 3939, y + 2939) * 10;
        if (fnlGetNoise2D(&w->rock, nx - 2939, ny + 9593) < -0.9) {
            block = "game:copper_ore";
        }
        if (fnlGetNoise2D(&w->rock, nx + 39032, ny - 2939) < -0.9) {
            block = "game:tin_ore";
        }
        if (fnlGetNoise2D(&w->rock, nx - 2939, ny + 69303) < -0.9) {
            block = "game:coal_ore";
        }
    }

    w->blocks[x][y] = shgeti(blocks, block);
    
    // double end = SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();

    // printf("%f\n", end - start);
}

void world_placelava(world* w) {
    int i = w->genIdx;

    w->genIdx++;

    if (i == w->genMax - 1) {
        w->finishedGen = true;
        w->genIdx = 0;
        w->genMax = 1;
        w->generateState = genLiquids;
        return;
    }

    int x = i % WORLD_WIDTH;
    int y = i / WORLD_WIDTH;

    if (w->blocks[x][y] == 0 && y < w->heightMap[x] && !contains(w->waterpos, x, y, shgeti(blocks, "game:water"))) {
        float chance = 2 * (WORLD_HEIGHT - y) / (float)WORLD_HEIGHT - 1;
        chance *= 0.02;
        
        if ((murmur_hash_combine(x, y) & 0xff) / 255.0 < chance) {
            struct ffp* points = floodfill(w, x, y, shgeti(blocks, "game:lava"));
            
            int miny = WORLD_HEIGHT;
            int minx = 0;
            
            for (int i = 0; i < arrlen(points); i++) {
                if (points[i].y < miny) {
                    miny = points[i].y;
                    minx = points[i].x;
                }
            }

            arrfree(points);

            arrput(w->fillWaterPos, ((struct ffp){minx, miny + 4, shgeti(blocks, "game:lava")}));
        }
    }
}

void world_fillliquids(world* w) {
    int i = w->genIdx;

    w->genIdx++;

    if (arrlen(w->fillWaterPos) == 0) {
        w->finishedGen = true;
        w->genIdx = 0;
        w->genMax = WORLD_WIDTH;
        w->generateState = genVegetation;
        arrfree(w->fillWaterPos);
        return;
    }
    
    struct ffp p = arrpop(w->fillWaterPos);
    w->genMax--;

    int x = p.x;
    int y = p.y;
    int bl = p.block;

    w->blocks[x][y] = bl;
    int b = world_getblock(w, x, y - 1);
    if (bl == shgeti(blocks, "game:water") && b == shgeti(blocks, "game:lava")) {
        w->blocks[x][y] = shgeti(blocks, "game:obsidian");
    }

    if (world_getblock(w, x + 1, y) == 0 && !contains(w->waterpos, x + 1, y, p.block)) {
        arrput(w->fillWaterPos, ((struct ffp){x + 1, y, p.block}));
        w->genMax++;
    }

    if (world_getblock(w, x - 1, y) == 0 && !contains(w->waterpos, x - 1, y, p.block)) {
        arrput(w->fillWaterPos, ((struct ffp){x - 1, y, p.block}));
        w->genMax++;
    }

    if (world_getblock(w, x, y - 1) == 0 && !contains(w->waterpos, x, y - 1, p.block)) {
        arrput(w->fillWaterPos, ((struct ffp){x, y - 1, p.block}));
        w->genMax++;
    }
}

void world_addvegetation(world* w) {
    int x = w->genIdx;

    w->genIdx++;

    if (x == w->genMax - 1) {
        w->finishedGen = true;
        w->genIdx = 0;
        w->genMax = WORLD_WIDTH * WORLD_HEIGHT;
        w->generateState = genData;
        arrfree(w->fillWaterPos);
        return;
    }

    int y = w->heightMap[x];
    int c = murmur_hash_combine(x, y) % 256;
    
    if (world_getblock(w, x, y) != shgeti(blocks, "game:grass"))
        return;
    
    if (c < 16) {
        world_setblock(w, x, y + 1, shgeti(blocks, "game:stump"));

        int height = murmur_hash_combine(x, y + 4593) % 6 + 6;
        for (int i = 0; i < height; i++) {
            world_setblock(w, x, y + i + 2, shgeti(blocks, "game:log"));
        }
    }
}

void world_updatedata(world* w, int x, int y) {
    uint32_t data = 0;
    int bl = world_getblock(w, x, y);
    block b = blocks[bl].value;
    blockshape shape = b.shape;
    if (shape == shape_edges) {
        int top = world_getblock(w, x, y + 1);
        int bottom = world_getblock(w, x, y - 1);
        int left = world_getblock(w, x - 1, y);
        int right = world_getblock(w, x + 1, y);
        int topleft = world_getblock(w, x - 1, y + 1);
        int topright = world_getblock(w, x + 1, y + 1);
        int bottomleft = world_getblock(w, x - 1, y - 1);
        int bottomright = world_getblock(w, x + 1, y - 1);
        data |= (!connectBlock(bl, top)) << 0;
        data |= (!connectBlock(bl, bottom)) << 1;
        data |= (!connectBlock(bl, left)) << 2;
        data |= (!connectBlock(bl, right)) << 3;
        data |= (!connectBlock(bl, topleft)) << 4;
        data |= (!connectBlock(bl, topright)) << 5;
        data |= (!connectBlock(bl, bottomleft)) << 6;
        data |= (!connectBlock(bl, bottomright)) << 7;
        data |= (blocks[top].value.shape != shape_edges) << 8;
        data |= (blocks[bottom].value.shape != shape_edges) << 9;
        data |= (blocks[left].value.shape != shape_edges) << 10;
        data |= (blocks[right].value.shape != shape_edges) << 11;
        data |= (blocks[topleft].value.shape != shape_edges) << 12;
        data |= (blocks[topright].value.shape != shape_edges) << 13;
        data |= (blocks[bottomleft].value.shape != shape_edges) << 14;
        data |= (blocks[bottomright].value.shape != shape_edges) << 15;
    } else if (shape == shape_liquid) {
        if (blocks[world_getblock(w, x, y + 1)].value.shape != shape_liquid) {
            data = 4;
        } else if (blocks[world_getblock(w, x, y + 2)].value.shape != shape_liquid) {
            data = 12;
        } else {
            data = 15;
        }
    }
    world_setdata(w, x, y, data);
}

void world_gendata(world* w) {
    int i = w->genIdx;

    w->genIdx++;

    if (i == WORLD_WIDTH * WORLD_HEIGHT - 1) {
        w->finishedGenData = true;
        w->genIdx = 0;
        w->generateState = genDone;
        return;
    }

    int x = i % WORLD_WIDTH;
    int y = i / WORLD_WIDTH;

    world_updatedata(w, x, y);
}

void world_gendatarange(world* w, int minx, int miny, int maxx, int maxy) {
    for (int y = miny; y < maxy; y++) {
        for (int x = minx; x < maxx; x++) {
            if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
                continue;
            world_updatedata(w, x, y);
        }
    }
}

uint8_t world_getblock(world* w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 1;
    return w->blocks[x][y] & 0xff;
}

uint32_t world_getdata(world* w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 0;
    return w->blocks[x][y] >> 8;
}

uint32_t world_getblockdata(world* w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 1;
    return w->blocks[x][y];
}

void world_setblock(world* w, int x, int y, uint8_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w->blocks[x][y] = w->blocks[x][y] & ~0xff | v;
}

void world_setdata(world* w, int x, int y, uint32_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w->blocks[x][y] = w->blocks[x][y] & 0xff | (v << 8);
}

void world_setblockdata(world* w, int x, int y, uint32_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w->blocks[x][y] = v;
}

void drawBlock(SDL_Renderer* renderer, struct blockhash* blocks, int b, int x, int y, float camx, float camy) {
    int data = b >> 8;
    int block = b & 0xff;

    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    SDL_Rect src;
    src.x = data * 8;
    src.y = 0;
    src.w = 8;
    src.h = 8;

    SDL_Rect dst;
    dst.x = (x * 8 - (int)(camx * 8)) * 2;
    dst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 16;
    dst.w = 16;
    dst.h = 16;

    SDL_RenderCopy(renderer, blocks[block].value.tex, &src, &dst);
}

void drawBlockLog(SDL_Renderer* renderer, struct blockhash* blocks, int b, int x, int y, float camx, float camy) {
    int data = b >> 8;
    int block = b & 0xff;

    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    int texRows;
    SDL_QueryTexture(blocks[block].value.tex, NULL, NULL, NULL, &texRows);
    texRows /= 8;

    int row = murmur_hash_combine(x, y) % texRows;

    SDL_Rect src;
    src.x = 0;
    src.y = row * 8;
    src.w = 24;
    src.h = 8;

    SDL_Rect dst;
    dst.x = (x * 8 - (int)(camx * 8)) * 2 - 16;
    dst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 16;
    dst.w = 48;
    dst.h = 16;

    SDL_RenderCopy(renderer, blocks[block].value.tex, &src, &dst);
}

void drawBlockEdges(SDL_Renderer* renderer, struct blockhash* blocks, int b, int x, int y, float camx, float camy) {
    int data = b >> 8;
    int block = b & 0xff;

    int top = (data >> 8) & 1;
    int bottom = (data >> 9) & 1;
    int left = (data >> 10) & 1;
    int right = (data >> 11) & 1;
    int topLeft = (data >> 12) & 1;
    int topRight = (data >> 13) & 1;
    int bottomLeft = (data >> 14) & 1;
    int bottomRight = (data >> 15) & 1;

    int topLeftTextureId = murmur_hash_combine(x, y) % 16;
    int topRightTextureId = murmur_hash_combine(x + 29402, y + 3092) % 16;
    int bottomLeftTextureId = murmur_hash_combine(x + 19163, y - 2939) % 16;
    int bottomRightTextureId = murmur_hash_combine(x - 29222, y + 23332) % 16;

    if (topLeft || (top && left)) {
        topLeftTextureId = 32;
    }

    if (topRight || (top && right)) {
        topRightTextureId = 33;
    }

    if (bottomLeft || (bottom && left)) {
        bottomLeftTextureId = 34;
    }

    if (bottomRight || (bottom && right)) {
        bottomRightTextureId = 35;
    }

    if (!top && !left && topLeft) {
        topLeftTextureId = 36;
    }

    if (!top && !right && topRight) {
        topRightTextureId = 37;
    }

    if (!bottom && !left && bottomLeft) {
        bottomLeftTextureId = 38;
    }

    if (!bottom && !right && bottomRight) {
        bottomRightTextureId = 39;
    }

    if (!left && top) {
        topLeftTextureId = 16 + murmur_hash_combine(x, y) % 4;
    }
    
    if (!right && top) {
        topRightTextureId = 16 + murmur_hash_combine(x, y) % 4;
    }

    if (!left && bottom) {
        bottomLeftTextureId = 20 + murmur_hash_combine(x + 29402, y + 3092) % 4;
    }
    
    if (!right && bottom) {
        bottomRightTextureId = 20 + murmur_hash_combine(x + 29402, y + 3092) % 4;
    }

    if (!top && left) {
        topLeftTextureId = 24 + murmur_hash_combine(x + 19163, y - 2939) % 4;
    }
    
    if (!bottom && left) {
        bottomLeftTextureId = 24 + murmur_hash_combine(x + 19163, y - 2939) % 4;
    }

    if (!top && right) {
        topRightTextureId = 28 + murmur_hash_combine(x - 29222, y + 23332) % 4;
    }
    
    if (!bottom && right) {
        bottomRightTextureId = 28 + murmur_hash_combine(x - 29222, y + 23332) % 4;
    }

    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    SDL_Rect topLeftSrc;
    topLeftSrc.x = topLeftTextureId * 4;
    topLeftSrc.y = 0;
    topLeftSrc.w = 4;
    topLeftSrc.h = 4;

    SDL_Rect topLeftDst;
    topLeftDst.x = (x * 8 - (int)(camx * 8)) * 2;
    topLeftDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 16;
    topLeftDst.w = 8;
    topLeftDst.h = 8;

    SDL_Rect topRightSrc;
    topRightSrc.x = topRightTextureId * 4;
    topRightSrc.y = 0;
    topRightSrc.w = 4;
    topRightSrc.h = 4;

    SDL_Rect topRightDst;
    topRightDst.x = (x * 8 - (int)(camx * 8)) * 2 + 8;
    topRightDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 16;
    topRightDst.w = 8;
    topRightDst.h = 8;

    SDL_Rect bottomLeftSrc;
    bottomLeftSrc.x = bottomLeftTextureId * 4;
    bottomLeftSrc.y = 0;
    bottomLeftSrc.w = 4;
    bottomLeftSrc.h = 4;

    SDL_Rect bottomLeftDst;
    bottomLeftDst.x = (x * 8 - (int)(camx * 8)) * 2;
    bottomLeftDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 8;
    bottomLeftDst.w = 8;
    bottomLeftDst.h = 8;

    SDL_Rect bottomRightSrc;
    bottomRightSrc.x = bottomRightTextureId * 4;
    bottomRightSrc.y = 0;
    bottomRightSrc.w = 4;
    bottomRightSrc.h = 4;

    SDL_Rect bottomRightDst;
    bottomRightDst.x = (x * 8 - (int)(camx * 8)) * 2 + 8;
    bottomRightDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 8;
    bottomRightDst.w = 8;
    bottomRightDst.h = 8;

    SDL_RenderCopy(renderer, blocks[block].value.tex, &topLeftSrc, &topLeftDst);
    SDL_RenderCopy(renderer, blocks[block].value.tex, &topRightSrc, &topRightDst);
    SDL_RenderCopy(renderer, blocks[block].value.tex, &bottomLeftSrc, &bottomLeftDst);
    SDL_RenderCopy(renderer, blocks[block].value.tex, &bottomRightSrc, &bottomRightDst);
}

void drawBlockEdgesFoliage(SDL_Renderer* renderer, struct blockhash* blocks, int b, int x, int y, float camx, float camy) {
    int data = b >> 8;
    int block = b & 0xff;

    int top = data & 1;
    int bottom = (data >> 1) & 1;
    int left = (data >> 2) & 1;
    int right = (data >> 3) & 1;
    int topLeft = (data >> 4) & 1;
    int topRight = (data >> 5) & 1;
    int bottomLeft = (data >> 6) & 1;
    int bottomRight = (data >> 7) & 1;
    
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    int texRows;
    SDL_QueryTexture(blocks[block].value.foliage, NULL, NULL, NULL, &texRows);
    texRows /= 4;

    if (top) {
        int tflIdX = murmur_hash_combine(x + 39494, y - 2939) % 6;
        int tflIdY = murmur_hash_combine(x + 3430, y - 22033) % texRows;

        int tfrIdX = murmur_hash_combine(x - 2939, y + 53939) % 6;
        int tfrIdY = murmur_hash_combine(x + 66904, y - 22293) % texRows;

        SDL_Rect tflSrc;
        tflSrc.x = tflIdX * 4;
        tflSrc.y = tflIdY * 4;
        tflSrc.w = 4;
        tflSrc.h = 4;

        SDL_Rect tflDst;
        tflDst.x = (x * 8 - (int)(camx * 8)) * 2;
        tflDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 24;
        tflDst.w = 8;
        tflDst.h = 8;

        SDL_Rect tfrSrc;
        tfrSrc.x = tfrIdX * 4;
        tfrSrc.y = tfrIdY * 4;
        tfrSrc.w = 4;
        tfrSrc.h = 4;

        SDL_Rect tfrDst;
        tfrDst.x = (x * 8 - (int)(camx * 8)) * 2 + 8;
        tfrDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 24;
        tfrDst.w = 8;
        tfrDst.h = 8;

        SDL_RenderCopy(renderer, blocks[block].value.foliage, &tflSrc, &tflDst);
        SDL_RenderCopy(renderer, blocks[block].value.foliage, &tfrSrc, &tfrDst);
    }

    if (bottom) {
        int bflIdX = murmur_hash_combine(x + 22344, y - 39393) % 6 + 12;
        int bflIdY = murmur_hash_combine(x + 39393, y - 2234) % texRows;

        int bfrIdX = murmur_hash_combine(x - 55533, y + 2282) % 6 + 12;
        int bfrIdY = murmur_hash_combine(x + 7764, y - 32929) % texRows;

        SDL_Rect bflSrc;
        bflSrc.x = bflIdX * 4;
        bflSrc.y = bflIdY * 4;
        bflSrc.w = 4;
        bflSrc.h = 4;

        SDL_Rect bflDst;
        bflDst.x = (x * 8 - (int)(camx * 8)) * 2;
        bflDst.y = height - (y * 8 - (int)(camy * 8)) * 2;
        bflDst.w = 8;
        bflDst.h = 8;

        SDL_Rect bfrSrc;
        bfrSrc.x = bfrIdX * 4;
        bfrSrc.y = bfrIdY * 4;
        bfrSrc.w = 4;
        bfrSrc.h = 4;

        SDL_Rect bfrDst;
        bfrDst.x = (x * 8 - (int)(camx * 8)) * 2 + 8;
        bfrDst.y = height - (y * 8 - (int)(camy * 8)) * 2;
        bfrDst.w = 8;
        bfrDst.h = 8;

        SDL_RenderCopy(renderer, blocks[block].value.foliage, &bflSrc, &bflDst);
        SDL_RenderCopy(renderer, blocks[block].value.foliage, &bfrSrc, &bfrDst);
    }

    if (left) {
        int tflIdX = murmur_hash_combine(x + 3442, y - 20303) % 6 + 18;
        int tflIdY = murmur_hash_combine(x + 2202, y - 22020) % texRows;

        int bflIdX = murmur_hash_combine(x - 7547, y + 22293) % 6 + 18;
        int bflIdY = murmur_hash_combine(x + 292, y - 50594) % texRows;

        SDL_Rect tflSrc;
        tflSrc.x = tflIdX * 4;
        tflSrc.y = tflIdY * 4;
        tflSrc.w = 4;
        tflSrc.h = 4;

        SDL_Rect tflDst;
        tflDst.x = (x * 8 - (int)(camx * 8)) * 2 - 8;
        tflDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 16;
        tflDst.w = 8;
        tflDst.h = 8;

        SDL_Rect bflSrc;
        bflSrc.x = bflIdX * 4;
        bflSrc.y = bflIdY * 4;
        bflSrc.w = 4;
        bflSrc.h = 4;

        SDL_Rect bflDst;
        bflDst.x = (x * 8 - (int)(camx * 8)) * 2 - 8;
        bflDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 8;
        bflDst.w = 8;
        bflDst.h = 8;

        SDL_RenderCopy(renderer, blocks[block].value.foliage, &tflSrc, &tflDst);
        SDL_RenderCopy(renderer, blocks[block].value.foliage, &bflSrc, &bflDst);
    }

    if (right) {
        int tfrIdX = murmur_hash_combine(x + 2202, y - 22020) % 6 + 6;
        int tfrIdY = murmur_hash_combine(x + 2233, y - 49329) % texRows;
        
        int bfrIdX = murmur_hash_combine(x - 5553, y + 3939) % 6 + 6;
        int bfrIdY = murmur_hash_combine(x + 2202, y - 5549) % texRows;

        SDL_Rect tfrSrc;
        tfrSrc.x = tfrIdX * 4;
        tfrSrc.y = tfrIdY * 4;
        tfrSrc.w = 4;
        tfrSrc.h = 4;

        SDL_Rect tfrDst;
        tfrDst.x = (x * 8 - (int)(camx * 8)) * 2 + 16;
        tfrDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 16;
        tfrDst.w = 8;
        tfrDst.h = 8;

        SDL_Rect bfrSrc;
        bfrSrc.x = bfrIdX * 4;
        bfrSrc.y = bfrIdY * 4;
        bfrSrc.w = 4;
        bfrSrc.h = 4;

        SDL_Rect bfrDst;
        bfrDst.x = (x * 8 - (int)(camx * 8)) * 2 + 16;
        bfrDst.y = height - (y * 8 - (int)(camy * 8)) * 2 - 8;
        bfrDst.w = 8;
        bfrDst.h = 8;

        SDL_RenderCopy(renderer, blocks[block].value.foliage, &tfrSrc, &tfrDst);
        SDL_RenderCopy(renderer, blocks[block].value.foliage, &bfrSrc, &bfrDst);
    }
}

void world_render_range(world* w, int minx, int maxx, int miny, int maxy, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    for (int x = minx; x < maxx; x++) {
        for (int y = miny; y < maxy; y++) {
            if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
                continue;

            int b = w->blocks[x][y];

            int block = b & 0xff;
            if (block == 0)
                continue;
            
            if (blocks[block].value.shape == shape_edges) {
                drawBlockEdges(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            } else if (blocks[block].value.shape == shape_log) {
                drawBlockLog(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            } else {
                drawBlock(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            }
        }
    }

    for (int x = minx; x < maxx; x++) {
        for (int y = miny; y < maxy; y++) {
            if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
                continue;
            
            int b = w->blocks[x][y];
            int block = b & 0xff;
            if (blocks[block].value.foliage == NULL)
                continue;
            
            if (blocks[block].value.shape == shape_edges) {
                drawBlockEdgesFoliage(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            }
        }
    }
}

void world_render(world* w, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    int minx = (int)floorf(camx - (float)width / 32);
    int miny = (int)floorf(camy - (float)height / 32);
    int maxx = (int)ceilf(camx + (float)width / 32);
    int maxy = (int)ceilf(camy + (float)height / 32);

    world_render_range(w, minx, maxx, miny, maxy, camx, camy, blocks, renderer);
}

void save_string(char* str, FILE* out) {
    if (fputc(strlen(str), out) == EOF) {
        fprintf(stderr, "Failed to write string length to file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    if (fwrite(str, 1, strlen(str), out) != strlen(str)) {
        fprintf(stderr, "Failed to write string to file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void save_number(int32_t num, FILE* out) {
    if (fwrite(&num, sizeof(int32_t), 1, out) != 1) {
        fprintf(stderr, "Failed to write number to file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void save_compressed_number(int32_t num, FILE* out) {
    while (num > 127) {
        uint8_t byte = (num & 0x7F) | 0x80;
        if (fwrite(&byte, sizeof(uint8_t), 1, out) != 1) {
            fprintf(stderr, "Failed to write compressed number to file: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        num >>= 7;
    }
    uint8_t byte = num & 0x7F;
    if (fwrite(&byte, sizeof(uint8_t), 1, out) != 1) {
        fprintf(stderr, "Failed to write compressed number to file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void world_save(world* w, FILE* out) {
    save_number(WORLD_WIDTH, out);
    save_number(WORLD_HEIGHT, out);

    save_number(shlen(blocks), out);
    for (int i = 0; i < shlen(blocks); i++) {
        save_string(blocks[i].key, out);
        save_number(i, out);
    }

    int last = w->blocks[0][0];
    int num = 0;
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            int block = w->blocks[x][y];
            if (block != last) {
                save_compressed_number(num, out);
                save_compressed_number(last, out);
                
                num = 0;
                last = block;
            }

            num++;
        }
    }
    if (num != 0) {
        save_compressed_number(num, out);
        save_compressed_number(last, out);
    }
}

char* read_string(FILE* in) {
    uint8_t length;
    if (fread(&length, sizeof(uint8_t), 1, in) != 1) {
        fprintf(stderr, "Failed reading string length in file: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    char* str = (char*)malloc(length + 1);
    if (fread(str, sizeof(char), length, in) != length) {
        fprintf(stderr, "Failed reading string in file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    str[length] = '\0';
    return str;
}

int32_t read_number(FILE* in) {
    int32_t num;
    if (fread(&num, sizeof(int32_t), 1, in) != 1) {
        fprintf(stderr, "Failed reading number in file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return num;
}

int32_t read_compressed_number(FILE* in) {
    int32_t num = 0;
    int shift = 0;
    int byte;

    while (1) {
        byte = fgetc(in);
        if (byte == EOF) {
            if (feof(in)) {
                fprintf(stderr, "Unexpected end of file while reading compressed number\n");
                exit(EXIT_FAILURE);
            } else {
                perror("Failed to read compressed number from file");
                exit(EXIT_FAILURE);
            }
        }

        num |= (byte & 0x7F) << shift;
        if (!(byte & 0x80)) {
            break;
        }
        shift += 7;
    }

    return num;
}

void world_load(world* w, FILE* in) {
    int width = read_number(in);
    int height = read_number(in);

    if (width != WORLD_WIDTH || height != WORLD_HEIGHT) {
        fprintf(stderr, "World dimensions do not match!\n");
        return;
    }

    int numBlocks = read_number(in);

    struct { int key; char* value; }* loadblocks = NULL;
    for (int i = 0; i < numBlocks; i++) {
        char* str = read_string(in);
        int id = read_number(in);

        hmput(loadblocks, id, str);
    }

    int i = 0;
    while (i < WORLD_WIDTH * WORLD_HEIGHT) {
        int num = read_compressed_number(in);
        int b = read_compressed_number(in);
        int block = b & 0xff;
        int data = b >> 8;
        char* key = hmget(loadblocks, block);
        block = shgeti(blocks, key) | data << 8;

        for (int j = 0; j < num; j++) {
            int x = i % WORLD_WIDTH;
            int y = i / WORLD_WIDTH;

            w->blocks[x][y] = block;

            i++;
        }
    }

    for (int i = 0; i < numBlocks; i++) {
        free(loadblocks[i].value);
    }

    hmfree(loadblocks);
}

void save_world_to_png(world* w, SDL_Renderer* renderer, char* out) {
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WORLD_WIDTH, WORLD_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_LockSurface(surface);

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            uint32_t b = w->blocks[x][y];
            uint32_t block = b & 0xff;
            
            SDL_Color* colors = blocks[block].value.colors;
            if (colors == NULL)
                continue;
            
            int rand = murmur_hash_combine(x, y) % arrlen(colors);
            SDL_Color color = colors[rand];

            uint32_t col = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);

            ((Uint32*)surface->pixels)[(surface->h - y - 1) * surface->w + x] = col;
        }
    }

    SDL_UnlockSurface(surface);
    IMG_SavePNG(surface, out);
}