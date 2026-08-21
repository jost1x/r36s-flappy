#pragma once

#include "game_logic.h"
#include "input_handler.h"
#include "ui_action.h"

UiAction resolveMenuAction(GameState state, int selection, const InputActions& actions);
int wrapMenuSelection(int selection, int itemCount, int direction);
