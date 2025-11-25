#ifndef TEXT_H
#define TEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class Text {
public:
    Text(const std::string& font_path, int font_size, const std::string& text, const SDL_Color& color);
    ~Text();

    void render(SDL_Renderer* renderer, int x, int y);
    void setText(const std::string& text);

private:
    TTF_Font* font;
    std::string text;
    SDL_Color color;
    SDL_Texture* texture;
};

#endif // TEXT_H
