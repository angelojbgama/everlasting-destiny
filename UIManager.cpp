#include "UIManager.h"
#include <iostream>
#include <algorithm>

namespace {
const int kSidebarPadding = 14;
const int kColumnSpacing = 14;
const int kSectionSpacing = 12;
const int kPanelInnerPadding = 8;
const int kControlSectionHeight = 132;
const int kAbilityButtonHeight = 32;
const int kAbilityButtonSpacing = 6;
const int kLogLineHeight = 18;
const int kTextLineHeight = 20;
const int kTileSectionHeight = 56;
const int kEntitySectionMinHeight = 120;
const int kAbilitySectionMinHeight = 100;
const int kMissionSectionMinHeight = 90;
const int kLogSectionMinHeight = 110;
}

UIManager::UIManager(int x, int y, int w, int h)
    : rect({x, y, w, h}),
      rollRequested(false),
      endRequested(false),
      pendingAction(UIActionType::None),
      pendingAbilityIndex(-1),
      font(nullptr) {
    SDL_Rect controlRect = controlArea();
    const int buttonHeight = 34;
    const int buttonGap = 10;
    const int contentWidth = controlRect.w - 2 * kPanelInnerPadding;
    const int xStart = controlRect.x + kPanelInnerPadding;
    int rowY = controlRect.y + kPanelInnerPadding + 26;

    int halfWidth = (contentWidth - buttonGap) / 2;
    rollDiceButton.reset(new Button(xStart, rowY, halfWidth, buttonHeight, "Rolar Dado"));
    endTurnButton.reset(new Button(xStart + halfWidth + buttonGap, rowY, halfWidth, buttonHeight, "Encerrar"));

    rowY += buttonHeight + buttonGap;
    int thirdWidth = (contentWidth - 2 * buttonGap) / 3;
    moveButton.reset(new Button(xStart, rowY, thirdWidth, buttonHeight, "Mover"));
    attackButton.reset(new Button(xStart + thirdWidth + buttonGap, rowY, thirdWidth, buttonHeight, "Atacar"));
    abilityButton.reset(new Button(xStart + 2 * (thirdWidth + buttonGap), rowY, thirdWidth, buttonHeight, "Habilidade"));

    rowY += buttonHeight + buttonGap;
    int wideWidth = (contentWidth - buttonGap) / 2;
    interactButton.reset(new Button(xStart, rowY, wideWidth, buttonHeight, "Interagir"));
    passButton.reset(new Button(xStart + wideWidth + buttonGap, rowY, wideWidth, buttonHeight, "Passar"));

    font = TTF_OpenFont("DejaVuSans.ttf", 16);
    if (!font) {
        std::cerr << "Failed to load UI font: " << TTF_GetError() << std::endl;
    }
}

UIManager::~UIManager() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

SDL_Rect UIManager::controlArea() const {
    SDL_Rect area;
    area.x = rect.x + kSidebarPadding;
    area.y = rect.y + kSidebarPadding;
    area.w = rect.w - 2 * kSidebarPadding;
    area.h = kControlSectionHeight;
    return area;
}

void UIManager::drawPanel(SDL_Renderer* renderer, const SDL_Rect& area) const {
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 220);
    SDL_RenderFillRect(renderer, &area);
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderDrawRect(renderer, &area);
}

void UIManager::layoutAbilityButtons(const SDL_Rect& abilityRect) {
    int buttonWidth = abilityRect.w - 2 * kPanelInnerPadding;
    if (buttonWidth <= 0) return;
    int y = abilityRect.y + kPanelInnerPadding + 24;
    for (auto& button : abilityButtons) {
        SDL_Rect buttonRect = {abilityRect.x + kPanelInnerPadding, y, buttonWidth, kAbilityButtonHeight};
        button->setRect(buttonRect);
        y += kAbilityButtonHeight + kAbilityButtonSpacing;
    }
}

void UIManager::drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) const {
    if (!font || text.empty()) {
        return;
    }
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void UIManager::drawSectionTitle(SDL_Renderer* renderer, const std::string& text, int x, int y) const {
    drawText(renderer, text, x, y, {255, 215, 0, 255});
}

void UIManager::render(SDL_Renderer* renderer,
                       const Entity* currentEntity,
                       const Mission& mission,
                       const std::vector<std::string>& logEntries,
                       const std::string& hoverText) {
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_Rect controlRect = controlArea();
    drawPanel(renderer, controlRect);
    drawSectionTitle(renderer, "Acoes", controlRect.x + kPanelInnerPadding, controlRect.y + 6);

    auto drawButtonLabel = [&](const std::unique_ptr<Button>& button, const std::string& label) {
        button->render(renderer);
        SDL_Rect buttonRect = button->getRect();
        drawText(renderer, label, buttonRect.x + 10, buttonRect.y + 8);
    };

    drawButtonLabel(rollDiceButton, "Rolar");
    drawButtonLabel(endTurnButton, "Encerrar");
    drawButtonLabel(moveButton, "Mover");
    drawButtonLabel(attackButton, "Atacar");
    drawButtonLabel(abilityButton, "Habilidade");
    drawButtonLabel(interactButton, "Interagir");
    drawButtonLabel(passButton, "Passar");

    const int columnsTop = controlRect.y + controlRect.h + kSectionSpacing;
    int columnWidth = (rect.w - (2 * kSidebarPadding + kColumnSpacing)) / 2;
    if (columnWidth < 80) {
        columnWidth = (rect.w - 2 * kSidebarPadding) / 2;
    }
    SDL_Rect leftColumn = {rect.x + kSidebarPadding, columnsTop, columnWidth, rect.h - columnsTop - kSidebarPadding};
    SDL_Rect rightColumn = {leftColumn.x + columnWidth + kColumnSpacing, columnsTop, columnWidth, leftColumn.h};

    const int entityLines = currentEntity ? 5 + static_cast<int>(currentEntity->getStatuses().size()) : 1;
    const int entityHeight = std::max(kEntitySectionMinHeight, kPanelInnerPadding * 2 + 24 + entityLines * kTextLineHeight);
    SDL_Rect entityRect = {leftColumn.x, leftColumn.y, columnWidth, entityHeight};
    drawPanel(renderer, entityRect);
    drawSectionTitle(renderer, "Personagem", entityRect.x + kPanelInnerPadding, entityRect.y + 6);
    int textY = entityRect.y + kPanelInnerPadding + 26;
    const int textLimit = entityRect.y + entityRect.h - kPanelInnerPadding - kTextLineHeight;
    auto drawEntityLine = [&](const std::string& line) {
        if (textY > textLimit) return;
        drawText(renderer, line, entityRect.x + kPanelInnerPadding, textY);
        textY += kTextLineHeight;
    };
    if (currentEntity) {
        drawEntityLine(currentEntity->getName());
        drawEntityLine("HP: " + std::to_string(currentEntity->getCurrentHP()) + "/" + std::to_string(currentEntity->getMaxHP()));
        drawEntityLine("Energia: " + std::to_string(currentEntity->getCurrentEnergy()) + "/" + std::to_string(currentEntity->getMaxEnergy()));
        drawEntityLine("AP: " + std::to_string(currentEntity->getActionPoints()));
        drawEntityLine("Nivel: " + std::to_string(currentEntity->getLevel()) + " (" +
                       std::to_string(currentEntity->getExperience()) + "/" + std::to_string(currentEntity->getExperienceToNext()) + " XP)");
        for (const auto& status : currentEntity->getStatuses()) {
            drawEntityLine("Status: " + status.id + " (" + std::to_string(status.remainingTurns) + ")");
        }
    } else {
        drawEntityLine("Nenhum personagem ativo");
    }

    int abilityHeight = kPanelInnerPadding * 2 + 24;
    if (abilityButtons.empty()) {
        abilityHeight += kTextLineHeight;
    } else {
        abilityHeight += static_cast<int>(abilityButtons.size()) * (kAbilityButtonHeight + kAbilityButtonSpacing);
    }
    abilityHeight = std::max(kAbilitySectionMinHeight, abilityHeight);
    SDL_Rect abilityRect = {leftColumn.x, entityRect.y + entityRect.h + kSectionSpacing, columnWidth, abilityHeight};
    drawPanel(renderer, abilityRect);
    drawSectionTitle(renderer, "Habilidades", abilityRect.x + kPanelInnerPadding, abilityRect.y + 6);
    layoutAbilityButtons(abilityRect);
    for (size_t i = 0; i < abilityButtons.size(); ++i) {
        abilityButtons[i]->render(renderer);
        drawText(renderer, abilityEntries[i].label, abilityButtons[i]->getRect().x + 8, abilityButtons[i]->getRect().y + 8);
    }
    if (abilityButtons.empty()) {
        drawText(renderer, "Sem habilidades disponiveis", abilityRect.x + kPanelInnerPadding, abilityRect.y + kPanelInnerPadding + 26);
    }

    int missionLines = std::max(1, static_cast<int>(mission.getObjectives().size()));
    int missionHeight = std::max(kMissionSectionMinHeight, kPanelInnerPadding * 2 + 24 + missionLines * kTextLineHeight);
    SDL_Rect missionRect = {rightColumn.x, rightColumn.y, columnWidth, missionHeight};
    drawPanel(renderer, missionRect);
    drawSectionTitle(renderer, "Missao", missionRect.x + kPanelInnerPadding, missionRect.y + 6);
    int missionTextY = missionRect.y + kPanelInnerPadding + 26;
    const int missionLimit = missionRect.y + missionRect.h - kPanelInnerPadding - kTextLineHeight;
    for (const auto& objective : mission.getObjectives()) {
        if (missionTextY > missionLimit) break;
        std::string label = (objective.completed ? "[OK] " : "[  ] ") + objective.definition.description;
        drawText(renderer, label, missionRect.x + kPanelInnerPadding, missionTextY);
        missionTextY += kTextLineHeight;
    }

    int logLines = std::min(8, static_cast<int>(logEntries.size()));
    int logHeight = kPanelInnerPadding * 2 + 24 + std::max(1, logLines) * kLogLineHeight;
    logHeight = std::max(kLogSectionMinHeight, logHeight);
    SDL_Rect logRect = {rightColumn.x, missionRect.y + missionRect.h + kSectionSpacing, columnWidth, logHeight};
    drawPanel(renderer, logRect);
    drawSectionTitle(renderer, "Log", logRect.x + kPanelInnerPadding, logRect.y + 6);
    int availableLines = (logRect.h - (kPanelInnerPadding * 2 + 24)) / kLogLineHeight;
    availableLines = std::max(1, availableLines);
    int startIndex = std::max(0, static_cast<int>(logEntries.size()) - availableLines);
    int logY = logRect.y + kPanelInnerPadding + 24;
    for (int i = startIndex; i < static_cast<int>(logEntries.size()); ++i) {
        drawText(renderer, logEntries[i], logRect.x + kPanelInnerPadding, logY);
        logY += kLogLineHeight;
        if (logY > logRect.y + logRect.h - kPanelInnerPadding - kLogLineHeight) {
            break;
        }
    }

    SDL_Rect tileRect = {rightColumn.x, logRect.y + logRect.h + kSectionSpacing, columnWidth, kTileSectionHeight};
    drawPanel(renderer, tileRect);
    drawSectionTitle(renderer, "Tile", tileRect.x + kPanelInnerPadding, tileRect.y + 6);
    drawText(renderer, hoverText, tileRect.x + kPanelInnerPadding, tileRect.y + kPanelInnerPadding + 24);
}

void UIManager::handleEvent(const SDL_Event& event) {
    if (event.type != SDL_MOUSEBUTTONDOWN) {
        return;
    }
    const int mouse_x = event.button.x;
    const int mouse_y = event.button.y;
    if (rollDiceButton->isClicked(mouse_x, mouse_y)) {
        rollRequested = true;
    } else if (endTurnButton->isClicked(mouse_x, mouse_y)) {
        endRequested = true;
    } else if (moveButton->isClicked(mouse_x, mouse_y)) {
        pendingAction = UIActionType::Move;
    } else if (attackButton->isClicked(mouse_x, mouse_y)) {
        pendingAction = UIActionType::Attack;
    } else if (abilityButton->isClicked(mouse_x, mouse_y)) {
        pendingAction = UIActionType::Ability;
    } else if (interactButton->isClicked(mouse_x, mouse_y)) {
        pendingAction = UIActionType::Interact;
    } else if (passButton->isClicked(mouse_x, mouse_y)) {
        pendingAction = UIActionType::Pass;
    } else {
        int index = 0;
        for (auto& button : abilityButtons) {
            if (button->isClicked(mouse_x, mouse_y)) {
                pendingAbilityIndex = index;
                pendingAction = UIActionType::Ability;
                break;
            }
            index++;
        }
    }
}

void UIManager::setAbilities(const std::vector<AbilityButtonEntry>& entries) {
    abilityEntries = entries;
    abilityButtons.clear();
    SDL_Rect placeholder = controlArea();
    placeholder.y += placeholder.h + kSectionSpacing;
    placeholder.h = kAbilityButtonHeight;
    int buttonWidth = std::max(80, placeholder.w - 2 * kPanelInnerPadding);
    for (const auto& entry : abilityEntries) {
        std::unique_ptr<Button> button(new Button(placeholder.x + kPanelInnerPadding,
                                                  placeholder.y,
                                                  buttonWidth,
                                                  kAbilityButtonHeight,
                                                  entry.label));
        abilityButtons.push_back(std::move(button));
        placeholder.y += kAbilityButtonHeight + kAbilityButtonSpacing;
    }
}

bool UIManager::consumeRollRequest() {
    if (rollRequested) {
        rollRequested = false;
        return true;
    }
    return false;
}

bool UIManager::consumeEndTurnRequest() {
    if (endRequested) {
        endRequested = false;
        return true;
    }
    return false;
}

UIActionType UIManager::consumeActionRequest() {
    UIActionType result = pendingAction;
    pendingAction = UIActionType::None;
    return result;
}

int UIManager::consumeAbilitySelection() {
    int result = pendingAbilityIndex;
    pendingAbilityIndex = -1;
    return result;
}
