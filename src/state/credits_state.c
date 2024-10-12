#include "credits_state.h"
#include <stdio.h>

#include "utils.h"

#include "state.h"

FILE* creditsFp;

void credits_state_init(void) {
    creditsFp = fopen("assets/credits.txt", "r");
}

void credits_state_cleanup(void) {
    fclose(creditsFp);
}

void credits_state_handle_events(SDL_Event e) {
    
}

void credits_state_update(SDL_Renderer* renderer, float delta) {

}

void credits_state_draw(SDL_Renderer* renderer) {
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    render_text(renderer, width / 2, 24, "Credits", 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_CENTER, YALIGN_BOTTOM);

    fseek(creditsFp, 0, SEEK_SET);

    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), creditsFp)) {
        int nl = strcspn(line, "\n");
        if (nl != strlen(line))
            line[nl] = 0;
        if(nl != 0)
            render_text(renderer, width / 2, i * 64 + 88, line, 48, (SDL_Color){255, 255, 255, 255}, font, XALIGN_CENTER, YALIGN_BOTTOM);
        i++;
    }
}
