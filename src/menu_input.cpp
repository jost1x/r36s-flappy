#include "menu_input.h"

UiAction resolveMenuAction(GameState state, int selection, const InputActions& actions) {
    if (state == GameState::Options) {
        if (actions.back || actions.pause || (actions.confirm && selection == 4)) return UiAction::Back;
        return UiAction::None;
    }
    if (state == GameState::Ready && actions.options) return UiAction::Options;
    if (state == GameState::Paused && (actions.back || actions.pause)) return UiAction::Continue;
    if (state == GameState::GameOver && actions.back) return UiAction::Menu;
    if (!actions.confirm) return UiAction::None;

    if (state == GameState::Ready) return selection == 0 ? UiAction::Play : UiAction::Options;
    if (state == GameState::Paused) {
        return selection == 0 ? UiAction::Continue : (selection == 1 ? UiAction::Restart : UiAction::ToggleMute);
    }
    if (state == GameState::GameOver) return selection == 0 ? UiAction::Retry : UiAction::Menu;
    return UiAction::None;
}

int wrapMenuSelection(int selection, int itemCount, int direction) {
    return (selection + direction + itemCount) % itemCount;
}
