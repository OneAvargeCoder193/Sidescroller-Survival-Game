#include "entity.h"

#include "stb_ds.h"

struct entitytypehash* entitytypes = NULL;
entity* entities = NULL;

int registerEntityType(const char* key, const cJSON* json, Assets *assets) {
    entitytype e;
    e.textures = NULL;

    cJSON* textures = cJSON_GetObjectItemCaseSensitive(json, "textures");
    for (int i = 0; i < cJSON_GetArraySize(textures); i++) {
        cJSON* t = cJSON_GetArrayItem(textures, i);
        SDL_Texture* tex = shget(assets->entityTextures, t->valuestring);
        arrput(e.textures, tex);
    }

    e.json = cJSON_Duplicate(json, 1);

    shput(entitytypes, key, e);

    return arrlen(entitytypes) - 1;
}

entity createEntity(const char* entitytype, float x, float y, float dx, float dy) {
    int type = shgeti(entitytypes, entitytype);
    entity e;
    e.type = type;
    e.vars = cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(entitytypes[type].value.json, "vars"), 1);
    e.x = x;
    e.y = y;
    e.dx = dx;
    e.dy = dy;
    return e;
}

void updateEntity(entity* e, float delta) {

}

void renderEntity(SDL_Renderer* renderer, entity e, float camx, float camy) {
    int frame = cJSON_GetObjectItemCaseSensitive(e.vars, "animframe")->valueint;
    int side = cJSON_GetObjectItemCaseSensitive(e.vars, "side")->valueint;

    float renx = floorf(e.x * 8) / 8;
    float reny = floorf(e.y * 8) / 8;

    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    SDL_Texture** textures = entitytypes[e.type].value.textures;

    SDL_Rect src;
    src.x = frame * 32;
    src.y = side * 32;
    src.w = 32;
    src.h = 32;

    SDL_Rect dst;
    dst.x = (renx - camx) * 16 + width / 2.0 - 32;
    dst.y = height - (reny - camy) * 16 - height / 2.0 - 32;
    dst.w = 64;
    dst.h = 64;

    // printf("%d %d %d %d\n", src.x, src.y, dst.x, dst.y);
    // printf("%f %f, %f %f\n", renx, reny, camx, camy);
    
    // for (int i = 0; i < arrlen(textures); i++) {
    SDL_RenderCopy(renderer, textures[0], &src, &dst);
    // }
}

void destroyEntityType(entitytype e) {
    cJSON_Delete(e.json);
    arrfree(e.textures);
}

void destroyEntity(entity e) {
    cJSON_Delete(e.vars);
}