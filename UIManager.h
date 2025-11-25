#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <vector>
#include <string>
#include "Button.h"
#include "Entity.h"
#include "Mission.h"

enum class UIActionType {
    None,
    Move,
    Attack,
    Ability,
    Interact,
    Pass
};

struct AbilityButtonEntry {
    std::string abilityId;
    std::string label;
};

class UIManager {
public:
    UIManager(int x, int y, int w, int h);
    ~UIManager();

    void render(SDL_Renderer* renderer,
                const Entity* currentEntity,
                const Mission& mission,
                const std::vector<std::string>& logEntries,
                const std::string& hoverText);

    void handleEvent(const SDL_Event& event);

    void setAbilities(const std::vector<AbilityButtonEntry>& entries);

    bool consumeRollRequest();
    bool consumeEndTurnRequest();
    UIActionType consumeActionRequest();
    int consumeAbilitySelection();

private:
    SDL_Rect rect;
    std::unique_ptr<Button> rollDiceButton;
    std::unique_ptr<Button> endTurnButton;
    std::unique_ptr<Button> moveButton;
    std::unique_ptr<Button> attackButton;
    std::unique_ptr<Button> abilityButton;
    std::unique_ptr<Button> interactButton;
    std::unique_ptr<Button> passButton;
    std::vector<std::unique_ptr<Button>> abilityButtons;
    std::vector<AbilityButtonEntry> abilityEntries;

    bool rollRequested;
    bool endRequested;
    UIActionType pendingAction;
    int pendingAbilityIndex;

    TTF_Font* font;

    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color = {255, 255, 255, 255}) const;
    void drawSectionTitle(SDL_Renderer* renderer, const std::string& text, int x, int y) const;
    void drawPanel(SDL_Renderer* renderer, const SDL_Rect& area) const;
    void layoutAbilityButtons(const SDL_Rect& abilityRect);
    SDL_Rect controlArea() const;
};

#endif
