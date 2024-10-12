#include "entity.h"

#include "stb_ds.h"

struct entitytypehash* entitytypes = NULL;
entity* entities = NULL;

comparison getComp(char* str) {
    if (strcmp(str, "less") == 0) {
        return less;
    } else if (strcmp(str, "greater") == 0) {
        return greater;
    }
    return -1;
}

int registerEntityType(const char* key, const cJSON* json, Assets *assets) {
    entitytype e;
    e.textures = NULL;

    cJSON* textures = cJSON_GetObjectItemCaseSensitive(json, "textures");
    cJSON* t;
    cJSON_ArrayForEach(t, textures) {
        SDL_Texture* tex = shget(assets->entityTextures, t->valuestring);
        arrput(e.textures, tex);
    }

    e.json = json;

    e.animations = NULL;

    cJSON* animations = cJSON_GetObjectItemCaseSensitive(json, "animations");
    cJSON* a;
    cJSON_ArrayForEach(a, animations) {
        animation anim;
        anim.start = cJSON_GetObjectItemCaseSensitive(a, "start")->valueint;
        anim.end = cJSON_GetObjectItemCaseSensitive(a, "end")->valueint;
        anim.speed = cJSON_GetObjectItemCaseSensitive(a, "speed")->valuedouble;
        shput(e.animations, a->string, anim);
    }

    e.components = NULL;

    cJSON* components = cJSON_GetObjectItemCaseSensitive(json, "components");
    cJSON* c;
    cJSON_ArrayForEach(c, components) {
        cJSON* tr = cJSON_GetObjectItemCaseSensitive(c, "trigger");
        trigger trig;
        trig.type = cJSON_GetObjectItemCaseSensitive(tr, "type")->valuestring;
        trig.comparison = getComp(cJSON_GetObjectItemCaseSensitive(tr, "comparison")->valuestring);
        trig.value = cJSON_GetObjectItemCaseSensitive(tr, "value")->valuedouble;
        
        cJSON* ac = cJSON_GetObjectItemCaseSensitive(c, "actions");
        action act;
        act.animation = cJSON_GetObjectItemCaseSensitive(ac, "animation")->valuestring;
        act.initial_state = cJSON_GetObjectItemCaseSensitive(ac, "initial_state")->valuestring;
        act.final_state = cJSON_GetObjectItemCaseSensitive(ac, "final_state")->valuestring;

        component c;
        c.trigger = trig;
        c.actions = act;
        arrput(e.components, c);
    }

    shput(entitytypes, key, e);

    return arrlen(entitytypes) - 1;
}

entity createEntity(const char* entitytype, float x, float y, float dx, float dy) {
    int type = shgeti(entitytypes, entitytype);
    entity e;
    e.type = type;
    e.x = x;
    e.y = y;
    e.dx = dx;
    e.dy = dy;
    e.animframe = 0;
    e.side = 0;
    e.triggercomponents = malloc(sizeof(int) * arrlen(entitytypes[type].value.components));
    return e;
}

void updateEntity(entity* e, float delta) {
    e->x += e->dx * delta;
    e->y += e->dy * delta;
}

void renderEntity(SDL_Renderer* renderer, entity e, float camx, float camy) {
    int frame = (int)e.animframe;
    int side = e.side;

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

    for (int i = 0; i < arrlen(textures); i++) {
        SDL_RenderCopy(renderer, textures[i], &src, &dst);
    }
}

void destroyEntityType(entitytype e) {
    shfree(e.animations);
    arrfree(e.components);
    arrfree(e.textures);
}

void destroyEntity(entity e) {
    free(e.triggercomponents);
}