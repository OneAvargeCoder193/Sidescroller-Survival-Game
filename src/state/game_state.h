#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "FastNoiseLite.h"

typedef enum entitystate {
    idle,
    walking,
    jumping,
    falling
} entitystate;

extern float camx;
extern float camy;

extern float playerx;
extern float playery;
extern float velx;
extern float vely;
extern int playerside;
extern float playerframe;
extern entitystate playerstate;
extern SDL_Texture* playerParts[6];
extern SDL_Texture* hat;

extern SDL_Texture* rainTex;

extern SDL_Texture* sun;
extern SDL_Texture* sunRing;

extern Mix_Music* music;

enum weather {
    weather_clear,
    weather_raining
};

extern enum weather weatherState;
extern float weatherTime;

struct rain {
    float x, y;
    float frame;
};

extern struct rain* raindrops;

void game_state_init(void);
void game_state_cleanup(void);
void game_state_handle_events(SDL_Event e);
void game_state_update(SDL_Renderer* renderer, float delta);
void game_state_draw(SDL_Renderer* renderer);

#endif // GAME_STATE_H