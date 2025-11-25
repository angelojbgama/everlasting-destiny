#include "Text.h"
#include <iostream>

Text::Text(const std::string& font_path, int font_size, const std::string& text_content, const SDL_Color& color_code)
    : font(nullptr), texture(nullptr), text(text_content), color(color_code) {
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }

    font = TTF_OpenFont(font_path.c_str(), font_size);
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
}

Text::~Text() {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

void Text::setText(const std::string& new_text) {
    if (new_text != text) {
        text = new_text;
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
}

void Text::render(SDL_Renderer* renderer, int x, int y) {
    if (font == nullptr) {
        return; // Font not loaded
    }

    if (texture == nullptr) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
        if (surface == nullptr) {
            std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return;
        }

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture == nullptr) {
            std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        }

        SDL_FreeSurface(surface);
    }

    int tex_w, tex_h;
    SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);
    SDL_Rect dst_rect = {x, y, tex_w, tex_h};
    SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
}
