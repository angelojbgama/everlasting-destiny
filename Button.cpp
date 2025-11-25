#include "Button.h"

Button::Button(int x, int y, int w, int h, const std::string& text)
    : rect({x, y, w, h}), text(text) {}

Button::~Button() {}

void Button::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Gray color for the button
    SDL_RenderFillRect(renderer, &rect);
}

bool Button::isClicked(int mouse_x, int mouse_y) {
    return mouse_x >= rect.x && mouse_x <= rect.x + rect.w &&
           mouse_y >= rect.y && mouse_y <= rect.y + rect.h;
}
