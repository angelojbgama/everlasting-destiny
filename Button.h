#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <string>

class Button {
public:
    Button(int x, int y, int w, int h, const std::string& text);
    ~Button();

    void render(SDL_Renderer* renderer);
    bool isClicked(int mouse_x, int mouse_y);
    const std::string& getText() const { return text; }
    void setText(const std::string& newText) { text = newText; }
    SDL_Rect getRect() const { return rect; }
    void setRect(const SDL_Rect& newRect) { rect = newRect; }

private:
    SDL_Rect rect;
    std::string text;
};

#endif // BUTTON_H
