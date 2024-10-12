#include "block.h"

#include "stb_ds.h"

struct blockhash* blocks = NULL;

block create_block(SDL_Texture* tex, SDL_Texture* foliage, bool transparent, blockshape shape) {
    block res;
    res.tex = tex;
    res.foliage = foliage;
    res.connects = NULL;
    res.shape = shape;
    res.collision = false;
    res.colors = NULL;
    return res;
}

blockshape string_to_blockshape(const char* str) {
    if (strcmp(str, "solid") == 0) {
        return shape_solid;
    } else if (strcmp(str, "edges") == 0) {
        return shape_edges;
    } else if (strcmp(str, "liquid") == 0) {
        return shape_liquid;
    } else if (strcmp(str, "log") == 0) {
        return shape_log;
    } else {
        fprintf(stderr, "Invalid blockshape: %s\n", str);
        return -1; // or handle error appropriately
    }
}

SDL_Color hexToColor(const char* hex) {
    SDL_Color color = {0, 0, 0, 255};  // Default color is black with full opacity

    if (hex[0] == '#') {
        unsigned int r, g, b;
        if (sscanf(hex + 1, "%02x%02x%02x", &r, &g, &b) == 3) {
            color.r = r;
            color.g = g;
            color.b = b;
        } else {
            fprintf(stderr, "Invalid color format\n");
        }
    } else {
        fprintf(stderr, "Color must start with #\n");
    }

    return color;
}

void registerBlock(const char* key, const cJSON* json, Assets *assets) {
    cJSON* textureJson = cJSON_GetObjectItemCaseSensitive(json, "texture");
    cJSON* foliageJson = cJSON_GetObjectItemCaseSensitive(json, "foliage");
    cJSON* shapeJson = cJSON_GetObjectItemCaseSensitive(json, "shape");
    cJSON* colorJson = cJSON_GetObjectItemCaseSensitive(json, "color");
    cJSON* collisionJson = cJSON_GetObjectItemCaseSensitive(json, "collision");

    SDL_Texture* tex = NULL;
    if (textureJson)
        tex = shget(assets->blockTextures, textureJson->valuestring);
    
    SDL_Texture* foliage = NULL;
    if (foliageJson)
        foliage = shget(assets->blockTextures, foliageJson->valuestring);
    
    SDL_Color* colors = NULL;
    if (colorJson) {
        for (int i = 0; i < cJSON_GetArraySize(colorJson); i++) {
            char* colorStr = cJSON_GetArrayItem(colorJson, i)->valuestring;
            arrput(colors, hexToColor(colorStr));
        }
    }
    
    blockshape shape = shape_solid;
    if (shapeJson)
        shape = string_to_blockshape(shapeJson->valuestring);
    
    bool collision = false;
    if (collisionJson)
        collision = collisionJson->valueint;
    
    block res;
    res.tex = tex;
    res.foliage = foliage;
    res.connects = NULL;
    res.shape = shape;
    res.collision = collision;
    res.colors = colors;

    shput(blocks, key, res);
}

void registerConnects(const char* key, const cJSON* json) {
    cJSON* connectsJson = cJSON_GetObjectItemCaseSensitive(json, "connects");

    int l = shlen(blocks);
    int* connects = malloc(l * sizeof(int));
    memset(connects, 0, l * sizeof(int));
    connects[shgeti(blocks, key)] = 1;
    if (connectsJson) {
        for (int i = 0; i < cJSON_GetArraySize(connectsJson); i++) {
            char* block = cJSON_GetArrayItem(connectsJson, i)->valuestring;
            int blockid = shgeti(blocks, block);
            connects[blockid] = 1;
        }
    }
    
    blocks[shgeti(blocks, key)].value.connects = connects;
}

void fixConnectConflict(const char* key) {
    if (blocks[shgeti(blocks, key)].value.shape == shape_edges)
    {
        for (int i = 0; i < shlen(blocks); i++) {
            if (blocks[i].value.shape == shape_edges && blocks[i].value.connects[shgeti(blocks, key)] == 0 && blocks[shgeti(blocks, key)].value.connects[i] == 0)
                blocks[shgeti(blocks, key)].value.connects[i] = 1;
        }
    }
}