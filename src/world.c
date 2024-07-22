#include "world.h"

#include "utils.h"

#define FNL_IMPL
#include "FastNoiseLite.h"

#include "stb_ds.h"

struct blockhash* blocks = NULL;

block create_block(SDL_Texture* tex, SDL_Texture* foliage, bool transparent, blockshape shape) {
    block res;
    res.tex = tex;
    res.foliage = foliage;
    // res.transparent = transparent;
    res.shape = shape;
    return res;
}

blockshape string_to_blockshape(const char* str) {
    if (strcmp(str, "solid") == 0) {
        return solid;
    } else if (strcmp(str, "edges") == 0) {
        return edges;
    } else if (strcmp(str, "liquid") == 0) {
        return liquid;
    } else {
        fprintf(stderr, "Invalid blockshape: %s\n", str);
        return -1; // or handle error appropriately
    }
}

void registerBlock(const char* key, const cJSON* json, Assets *assets) {
    cJSON* textureJson = cJSON_GetObjectItemCaseSensitive(json, "texture");
    cJSON* foliageJson = cJSON_GetObjectItemCaseSensitive(json, "foliage");
    cJSON* shapeJson = cJSON_GetObjectItemCaseSensitive(json, "shape");
    cJSON* innerBordersJson = cJSON_GetObjectItemCaseSensitive(json, "innerBorders");

    SDL_Texture* tex = NULL;
    if (textureJson)
        tex = shget(assets->blockTextures, textureJson->valuestring);
    
    SDL_Texture* foliage = NULL;
    if (foliageJson)
        foliage = shget(assets->blockTextures, foliageJson->valuestring);
    
    blockshape shape = solid;
    if (shapeJson)
        shape = string_to_blockshape(shapeJson->valuestring);
    
    bool innerBorders = false;
    if (innerBordersJson)
        innerBorders = innerBordersJson->valueint;
    
    block res;
    res.tex = tex;
    res.foliage = foliage;
    res.connects = NULL;
    res.shape = shape;
    res.innerBorders = innerBorders;

    // blocks[shgeti(blocks, key)].value = res;

    shput(blocks, key, res);
}

void registerConnects(const char* key, const cJSON* json) {
    cJSON* connectsJson = cJSON_GetObjectItemCaseSensitive(json, "connects");
    cJSON* connectsAllJson = cJSON_GetObjectItemCaseSensitive(json, "connects_all");

    bool connects_all = false;
    if (connectsAllJson)
        connects_all = connectsAllJson->valueint;
    
    int l = shlen(blocks);
    int* connects = malloc(l * sizeof(int));
    memset(connects, connects_all, l * sizeof(int));
    connects[0] = 0;
    connects[shgeti(blocks, key)] = 1;
    if (connectsJson) {
        for (int i = 0; i < cJSON_GetArraySize(connectsJson); i++) {
            char* block = cJSON_GetArrayItem(connectsJson, i)->valuestring;
            int blockid = shgeti(blocks, block);
            connects[blockid] = 1;
        }
    } else if (connects_all) {
        for (int i = 0; i < shlen(blocks); i++) {
            if (blocks[i].value.shape != edges)
                connects[i] = 0;
        }
    }

    blocks[shgeti(blocks, key)].value.connects = connects;
}

void fixConnectConflict(const char* key) {
    if (blocks[shgeti(blocks, key)].value.shape == edges)
    {
        for (int i = 0; i < shlen(blocks); i++) {
            if (blocks[i].value.shape == edges && blocks[i].value.connects[shgeti(blocks, key)] == 0 && blocks[shgeti(blocks, key)].value.connects[i] == 0)
                blocks[shgeti(blocks, key)].value.connects[i] = 1;
        }
    }
}

struct ffp {
    int x; int y; int block;
};

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

struct ffp* floodfill(world w, int x, int y, int block) {
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

    fnl_state n = fnlCreateState();
    n.fractal_type = FNL_FRACTAL_FBM;
    n.octaves = 16;
    
    fnl_state rock = fnlCreateState();
    rock.noise_type = FNL_NOISE_CELLULAR;
    rock.cellular_distance_func = FNL_CELLULAR_DISTANCE_EUCLIDEAN;

    struct ffp* fillWaterPos = NULL;
    struct ffp* waterpos = NULL;

    float h[WORLD_WIDTH];
    for (int x = 0; x < WORLD_WIDTH; x++) {
        h[x] = fnlGetNoise2D(&n, x / 2.0, 0) * 30 + WORLD_HEIGHT / 2;
    }

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            float height = h[x];
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
            if (y < height - 50) {
                block = "game:stone";
            }
            if (strcmp(block, "game:air") == 0 && y == WORLD_HEIGHT / 2) {
                arrput(fillWaterPos, ((struct ffp){x, y, shgeti(blocks, "game:water")}));
                arrput(waterpos, ((struct ffp){x, y, shgeti(blocks, "game:water")}));
            }
            if (fabsf(fnlGetNoise2D(&n, x, y)) < 0.2 && fabsf(fnlGetNoise2D(&n, x + 5399, y + 3494)) < 0.2) {
                block = "game:air";
            }
            if (strcmp(block, "game:dirt") == 0) {
                int nx = x + fnlGetNoise2D(&n, x - 2283, y - 9872) * 50;
                int ny = y + fnlGetNoise2D(&n, x + 3939, y + 2939) * 50;
                if (fnlGetNoise2D(&rock, nx, ny) < -0.5 && y < height - 7) {
                    block = "game:stone";
                }
            }

            if (strcmp(block, "game:stone") == 0) {
                int nx = x + fnlGetNoise2D(&n, x - 2283, y - 9872) * 10;
                int ny = y + fnlGetNoise2D(&n, x + 3939, y + 2939) * 10;
                if (fnlGetNoise2D(&rock, nx - 2939, ny + 9593) < -0.9) {
                    block = "game:copper_ore";
                }
                if (fnlGetNoise2D(&rock, nx + 39032, ny - 2939) < -0.9) {
                    block = "game:tin_ore";
                }
            }
            w.blocks[x][y] = shgeti(blocks, block);
        }
    }

    while (arrlen(fillWaterPos) != 0) {
        struct ffp p = arrpop(fillWaterPos);
        int x = p.x;
        int y = p.y;

        if (world_getblock(w, x + 1, y) == 0 && !contains(waterpos, x + 1, y, p.block)) {
            arrput(fillWaterPos, ((struct ffp){x + 1, y, p.block}));
            arrput(waterpos, ((struct ffp){x + 1, y, p.block}));
        }

        if (world_getblock(w, x - 1, y) == 0 && !contains(waterpos, x - 1, y, p.block)) {
            arrput(fillWaterPos, ((struct ffp){x - 1, y, p.block}));
            arrput(waterpos, ((struct ffp){x - 1, y, p.block}));
        }

        if (world_getblock(w, x, y - 1) == 0 && !contains(waterpos, x, y - 1, p.block)) {
            arrput(fillWaterPos, ((struct ffp){x, y - 1, p.block}));
            arrput(waterpos, ((struct ffp){x, y - 1, p.block}));
        }
    }
    
    arrfree(fillWaterPos);

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT / 2; y++) {
            if (w.blocks[x][y] == 0 && y < h[x] && !contains(waterpos, x, y, shgeti(blocks, "game:water"))) {
                float chance = (WORLD_HEIGHT - y - 1) / (float)WORLD_HEIGHT;
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

                    struct ffp* bs = floodfill(w, minx, miny + 4, shgeti(blocks, "game:lava"));
                    for (int i = 0; i < arrlen(bs); i++) {
                        w.blocks[bs[i].x][bs[i].y] = bs[i].block;
                    }
                    arrfree(bs);
                }
            }
        }
    }

    for (int i = 0; i < arrlen(waterpos); i++) {
        if (world_getblock(w, waterpos[i].x, waterpos[i].y) == 0) {
            w.blocks[waterpos[i].x][waterpos[i].y] = waterpos[i].block;
            if (world_getblock(w, waterpos[i].x, waterpos[i].y - 1) == shgeti(blocks, "game:lava")) {
                w.blocks[waterpos[i].x][waterpos[i].y] = shgeti(blocks, "game:obsidian");
            }
        }
    }
    arrfree(waterpos);

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            uint32_t data = world_getdata(w, x, y);
            int bl = world_getblock(w, x, y);
            block b = blocks[bl].value;
            blockshape shape = b.shape;
            if (shape == edges) {
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
                data |= (blocks[top].value.shape != edges) << 8;
                data |= (blocks[bottom].value.shape != edges) << 9;
                data |= (blocks[left].value.shape != edges) << 10;
                data |= (blocks[right].value.shape != edges) << 11;
                data |= (blocks[topleft].value.shape != edges) << 12;
                data |= (blocks[topright].value.shape != edges) << 13;
                data |= (blocks[bottomleft].value.shape != edges) << 14;
                data |= (blocks[bottomright].value.shape != edges) << 15;
            } else if (shape == liquid) {
                if (blocks[world_getblock(w, x, y + 1)].value.shape != liquid) {
                    data = 4;
                } else if (blocks[world_getblock(w, x, y + 2)].value.shape != liquid) {
                    data = 12;
                } else {
                    data = 15;
                }
            }
            w.blocks[x][y] |= data << 8;
        }
    }

    return w;
}


uint8_t world_getblock(world w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 1;
    return w.blocks[x][y] & 0xff;
}

uint32_t world_getdata(world w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 0;
    return w.blocks[x][y] >> 8;
}

uint32_t world_getblockdata(world w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 1;
    return w.blocks[x][y];
}

void world_setblock(world w, int x, int y, uint8_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w.blocks[x][y] = w.blocks[x][y] & ~0xff | v;
}

void world_setdata(world w, int x, int y, uint32_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w.blocks[x][y] = w.blocks[x][y] & 0xff | (v << 8);
}

void world_setblockdata(world w, int x, int y, uint32_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w.blocks[x][y] = v;
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

void drawBlockEdges(SDL_Renderer* renderer, struct blockhash* blocks, int b, int x, int y, float camx, float camy) {
    int data = b >> 8;
    int block = b & 0xff;

    if (blocks[block].value.innerBorders) {
        data <<= 8;
    }

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

void world_render_range(world w, int minx, int maxx, int miny, int maxy, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    for (int x = minx; x < maxx; x++) {
        for (int y = miny; y < maxy; y++) {
            int b = w.blocks[x][y];
            int block = b & 0xff;
            if (block == 0)
                continue;
            
            if (blocks[block].value.shape == edges) {
                drawBlockEdges(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            } else {
                drawBlock(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            }
        }
    }

    for (int x = minx; x < maxx; x++) {
        for (int y = miny; y < maxy; y++) {
            int b = w.blocks[x][y];
            int block = b & 0xff;
            if (blocks[block].value.foliage == NULL)
                continue;
            
            if (blocks[block].value.shape == edges) {
                drawBlockEdgesFoliage(renderer, blocks, b, x, y, camx - (float)width / 32, camy - (float)height / 32);
            }
        }
    }
}

void world_render(world w, float camx, float camy, struct blockhash* blocks, SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    int minx = (int)floorf(camx - (float)width / 32);
    int miny = (int)floorf(camy - (float)height / 32);
    int maxx = (int)ceilf(camx + (float)width / 32);
    int maxy = (int)ceilf(camy + (float)height / 32);

    world_render_range(w, minx, maxx, miny, maxy, camx, camy, blocks, renderer);
}