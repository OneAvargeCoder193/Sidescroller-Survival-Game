#include "world.h"

#include "utils.h"

#define FNL_IMPL
#include "FastNoiseLite.h"

block create_block(SDL_Texture* tex, SDL_Texture* foliage, bool transparent, bool hasEdges) {
    return (block){tex, foliage, transparent, hasEdges};
}

world world_init(block* blocks) {
    world w = {0};

    fnl_state n = fnlCreateState();
    fnl_state rock = fnlCreateState();
    rock.noise_type = FNL_NOISE_CELLULAR;
    rock.cellular_distance_func = FNL_CELLULAR_DISTANCE_EUCLIDEAN;
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            float height = fnlGetNoise2D(&n, x * 3, 0) * 5 + 15;
            int block = 0;
            if (y <= 15) {
                block = 3;
                if (y == 15) {
                    block |= 4 << 8;
                } else if (y == 14) {
                    block |= 12 << 8;
                } else {
                    block |= 15 << 8;
                }
            }
            if (fabsf(fnlGetNoise2D(&n, x * 3, y * 3)) > 0.4 || fabsf(fnlGetNoise2D(&n, x * 3 + 53, y * 3)) > 0.4) {
                if (y >= 15)
                {
                    if (y < height) {
                        block = 1;
                    }
                    if (y < height - 3) {
                        block = 2;
                    }
                    if (y < height - 15) {
                        block = 4;
                    }
                } else {
                    block = 2;
                }
            }
            if (block == 2) {
                if (fnlGetNoise2D(&rock, x * 3, y * 3) < -0.5) {
                    block = 4;
                }
            }
            w.blocks[x][y] = block;
        }
    }

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            uint8_t data = world_getdata(w, x, y);
            if (!blocks[world_getblock(w, x, y)].transparent) {
                data |= (blocks[world_getblock(w, x, y + 1)].transparent);
                data |= (blocks[world_getblock(w, x, y - 1)].transparent) << 1;
                data |= (blocks[world_getblock(w, x - 1, y)].transparent) << 2;
                data |= (blocks[world_getblock(w, x + 1, y)].transparent) << 3;
                data |= (blocks[world_getblock(w, x - 1, y + 1)].transparent) << 4;
                data |= (blocks[world_getblock(w, x + 1, y + 1)].transparent) << 5;
                data |= (blocks[world_getblock(w, x - 1, y - 1)].transparent) << 6;
                data |= (blocks[world_getblock(w, x + 1, y - 1)].transparent) << 7;
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

uint8_t world_getdata(world w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 0;
    return w.blocks[x][y] >> 8;
}

uint16_t world_getblockdata(world w, int x, int y) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return 1;
    return w.blocks[x][y];
}

void world_setblock(world w, int x, int y, uint8_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w.blocks[x][y] = w.blocks[x][y] & ~0xff | v;
}

void world_setdata(world w, int x, int y, uint8_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w.blocks[x][y] = w.blocks[x][y] & 0xff | (v << 8);
}

void world_setblockdata(world w, int x, int y, uint16_t v) {
    if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT)
        return;
    w.blocks[x][y] = v;
}

void drawBlock(SDL_Renderer* renderer, block* blocks, int b, int x, int y, float camx, float camy) {
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
    dst.x = (x - camx) * 24;
    dst.y = height - (y - camy) * 24 - 24;
    dst.w = 24;
    dst.h = 24;

    SDL_RenderCopy(renderer, blocks[block].tex, &src, &dst);
}

void drawBlockEdges(SDL_Renderer* renderer, block* blocks, int b, int x, int y, float camx, float camy) {
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

    int topLeftTextureId = murmur_hash_combine(x, y) % 16;
    int topRightTextureId = murmur_hash_combine(x + 29402, y + 3092) % 16;
    int bottomLeftTextureId = murmur_hash_combine(x + 19243, y - 2939) % 16;
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
        topLeftTextureId = 24 + murmur_hash_combine(x + 19243, y - 2939) % 4;
    }
    
    if (!bottom && left) {
        bottomLeftTextureId = 24 + murmur_hash_combine(x + 19243, y - 2939) % 4;
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
    topLeftDst.x = (x - camx) * 24;
    topLeftDst.y = height - (y - camy) * 24 - 24;
    topLeftDst.w = 12;
    topLeftDst.h = 12;

    SDL_Rect topRightSrc;
    topRightSrc.x = topRightTextureId * 4;
    topRightSrc.y = 0;
    topRightSrc.w = 4;
    topRightSrc.h = 4;

    SDL_Rect topRightDst;
    topRightDst.x = (x - camx) * 24 + 12;
    topRightDst.y = height - (y - camy) * 24 - 24;
    topRightDst.w = 12;
    topRightDst.h = 12;

    SDL_Rect bottomLeftSrc;
    bottomLeftSrc.x = bottomLeftTextureId * 4;
    bottomLeftSrc.y = 0;
    bottomLeftSrc.w = 4;
    bottomLeftSrc.h = 4;

    SDL_Rect bottomLeftDst;
    bottomLeftDst.x = (x - camx) * 24;
    bottomLeftDst.y = height - (y - camy) * 24 - 12;
    bottomLeftDst.w = 12;
    bottomLeftDst.h = 12;

    SDL_Rect bottomRightSrc;
    bottomRightSrc.x = bottomRightTextureId * 4;
    bottomRightSrc.y = 0;
    bottomRightSrc.w = 4;
    bottomRightSrc.h = 4;

    SDL_Rect bottomRightDst;
    bottomRightDst.x = (x - camx) * 24 + 12;
    bottomRightDst.y = height - (y - camy) * 24 - 12;
    bottomRightDst.w = 12;
    bottomRightDst.h = 12;

    SDL_RenderCopy(renderer, blocks[block].tex, &topLeftSrc, &topLeftDst);
    SDL_RenderCopy(renderer, blocks[block].tex, &topRightSrc, &topRightDst);
    SDL_RenderCopy(renderer, blocks[block].tex, &bottomLeftSrc, &bottomLeftDst);
    SDL_RenderCopy(renderer, blocks[block].tex, &bottomRightSrc, &bottomRightDst);
}

void drawBlockEdgesFoliage(SDL_Renderer* renderer, block* blocks, int b, int x, int y, float camx, float camy) {
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
    SDL_QueryTexture(blocks[block].foliage, NULL, NULL, NULL, &texRows);
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
        tflDst.x = (x - camx) * 24;
        tflDst.y = height - (y - camy) * 24 - 36;
        tflDst.w = 12;
        tflDst.h = 12;

        SDL_Rect tfrSrc;
        tfrSrc.x = tfrIdX * 4;
        tfrSrc.y = tfrIdY * 4;
        tfrSrc.w = 4;
        tfrSrc.h = 4;

        SDL_Rect tfrDst;
        tfrDst.x = (x - camx) * 24 + 12;
        tfrDst.y = height - (y - camy) * 24 - 36;
        tfrDst.w = 12;
        tfrDst.h = 12;

        SDL_RenderCopy(renderer, blocks[block].foliage, &tflSrc, &tflDst);
        SDL_RenderCopy(renderer, blocks[block].foliage, &tfrSrc, &tfrDst);
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
        bflDst.x = (x - camx) * 24;
        bflDst.y = height - (y - camy) * 24;
        bflDst.w = 12;
        bflDst.h = 12;

        SDL_Rect bfrSrc;
        bfrSrc.x = bfrIdX * 4;
        bfrSrc.y = bfrIdY * 4;
        bfrSrc.w = 4;
        bfrSrc.h = 4;

        SDL_Rect bfrDst;
        bfrDst.x = (x - camx) * 24 + 12;
        bfrDst.y = height - (y - camy) * 24;
        bfrDst.w = 12;
        bfrDst.h = 12;

        SDL_RenderCopy(renderer, blocks[block].foliage, &bflSrc, &bflDst);
        SDL_RenderCopy(renderer, blocks[block].foliage, &bfrSrc, &bfrDst);
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
        tflDst.x = (x - camx) * 24 - 12;
        tflDst.y = height - (y - camy) * 24 - 24;
        tflDst.w = 12;
        tflDst.h = 12;

        SDL_Rect bflSrc;
        bflSrc.x = bflIdX * 4;
        bflSrc.y = bflIdY * 4;
        bflSrc.w = 4;
        bflSrc.h = 4;

        SDL_Rect bflDst;
        bflDst.x = (x - camx) * 24 - 12;
        bflDst.y = height - (y - camy) * 24 - 12;
        bflDst.w = 12;
        bflDst.h = 12;

        SDL_RenderCopy(renderer, blocks[block].foliage, &tflSrc, &tflDst);
        SDL_RenderCopy(renderer, blocks[block].foliage, &bflSrc, &bflDst);
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
        tfrDst.x = (x - camx) * 24 + 24;
        tfrDst.y = height - (y - camy) * 24 - 24;
        tfrDst.w = 12;
        tfrDst.h = 12;

        SDL_Rect bfrSrc;
        bfrSrc.x = bfrIdX * 4;
        bfrSrc.y = bfrIdY * 4;
        bfrSrc.w = 4;
        bfrSrc.h = 4;

        SDL_Rect bfrDst;
        bfrDst.x = (x - camx) * 24 + 24;
        bfrDst.y = height - (y - camy) * 24 - 12;
        bfrDst.w = 12;
        bfrDst.h = 12;

        SDL_RenderCopy(renderer, blocks[block].foliage, &tfrSrc, &tfrDst);
        SDL_RenderCopy(renderer, blocks[block].foliage, &bfrSrc, &bfrDst);
    }
}

void world_render(world w, float camx, float camy, block* blocks, SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    int minx = (int)floorf(camx);
    int miny = (int)floorf(camy);
    int maxx = (int)ceilf(camx + (float)width / 24);
    int maxy = (int)ceilf(camy + (float)height / 24);
    for (int x = minx; x < maxx; x++) {
        for (int y = miny; y < maxy; y++) {
            int b = w.blocks[x][y];
            int block = b & 0xff;
            if (block == 0)
                continue;
            
            if (blocks[block].hasEdges) {
                drawBlockEdges(renderer, blocks, b, x, y, camx, camy);
            } else {
                drawBlock(renderer, blocks, b, x, y, camx, camy);
            }
        }
    }

    for (int x = minx; x < maxx; x++) {
        for (int y = miny; y < maxy; y++) {
            int b = w.blocks[x][y];
            int block = b & 0xff;
            if (blocks[block].foliage == NULL)
                continue;
            
            if (blocks[block].hasEdges) {
                drawBlockEdgesFoliage(renderer, blocks, b, x, y, camx, camy);
            }
        }
    }
}