#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL.h>

#include "cJSON.h"
#include "assets.h"

typedef struct animation {
    int start;
    int end;
    float speed;
} animation;

typedef enum comparison {
    greater,
    less
} comparison;

typedef struct trigger {
    char* type;
    comparison comparison;
    float value;
} trigger;

typedef struct action {
    char *animation;
    char *initial_state;
    char *final_state;
} action;

typedef struct component {
    trigger trigger;
    action actions;
} component;

struct animhash {
    char* key;
    animation value;
};

typedef struct entitytype {
    SDL_Texture** textures;
    struct animhash* animations;
    component* components;
    cJSON* json;
} entitytype;

typedef struct entity {
    int type;
    float animframe;

    int* triggercomponents;

    int side;
    float timer;
    float x;
    float y;
    float dx;
    float dy;
} entity;

struct entitytypehash {
    const char* key;
    entitytype value;
};

extern struct entitytypehash* entitytypes;
extern entity* entities;

int registerEntityType(const char* key, const cJSON* json, Assets *assets);

entity createEntity(const char* entitytype, float x, float y, float dx, float dy);

void updateEntity(entity* e, float delta);

void renderEntity(SDL_Renderer* renderer, entity e, float camx, float camy);

void destroyEntityType(entitytype e);

void destroyEntity(entity e);

#endif