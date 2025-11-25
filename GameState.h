#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class GameState {
    AwaitingRoll,
    ActionSelection,
    EnemyTurn,
    ResolvingAction,
    GameOver
};

#endif
