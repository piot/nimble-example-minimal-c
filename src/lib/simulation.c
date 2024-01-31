/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <example/game.h>
#include <example/input.h>
#include <example/simulation.h>

#define WIDTH 40
#define HEIGHT 30

static void snakeMoveWithFront(ExampleSnake* snake)
{
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }
}

static void snakeSetDirectionFromInput(ExampleSnake* snake, const ExamplePlayerInGameInput* pad)
{
    if (pad->horizontalAxis < 0) {
        snake->movementDirection = ExampleDirectionLeft;
    } else if (pad->horizontalAxis > 0) {
        snake->movementDirection = ExampleDirectionRight;
    } else if (pad->verticalAxis < 0) {
        snake->movementDirection = ExampleDirectionDown;
    } else if (pad->verticalAxis > 0) {
        snake->movementDirection = ExampleDirectionUp;
    }
}

static void snakeMoveFrontUsingDirection(ExampleSnake* snake)
{
    switch (snake->movementDirection) {
    case ExampleDirectionDown:
        snake->y[0]--;
        break;
    case ExampleDirectionUp:
        snake->y[0]++;
        break;
    case ExampleDirectionLeft:
        snake->x[0]--;
        break;
    case ExampleDirectionRight:
        snake->x[0]++;
        break;
    }
}

static uint32_t pseudoRandomNext(uint32_t* seed)
{
    const uint32_t c = 1013904223;
    const uint32_t a = 1664525;
    *seed = (*seed * a + c);
    return *seed;
}

static uint8_t spawnAvatar(ExampleGame* self)
{
    uint8_t snakeIndex = self->snakeCount;
    ExampleSnake* snake = &self->snakes[snakeIndex];
    snake->x[0] = 2;
    snake->y[0] = 2;
    snake->length = 1;
    self->snakeCount++;
    return snakeIndex;
}

/*
static uint8_t spawnPlayer(ExampleGame* self)
{
    (void)self;
    uint8_t playerIndex = self->playerCount;
    ExamplePlayer* player = &self->players[self->playerCount];

    self->playerCount++;

    return playerIndex;
}
*/

static void handleInGameInput(ExampleGame* self, const ExamplePlayerInput* playerInput)
{
    ExamplePlayer* player = &self->players[playerInput->participantId];
    CLOG_ASSERT(player->snakeIndex != EXAMPLE_ILLEGAL_INDEX, "illegal snake index");
    ExampleSnake* snake = &self->snakes[player->snakeIndex];
    snakeSetDirectionFromInput(snake, &playerInput->input.inGameInput);
}

static void spawnAvatarForPlayer(ExampleGame* self, ExamplePlayer* player)
{
    if (player->snakeIndex != EXAMPLE_ILLEGAL_INDEX) {
        player->snakeIndex = spawnAvatar(self);
    }
}

static void handleInput(ExampleGame* self, const ExamplePlayerInput* playerInput)
{
    CLOG_DEBUG("handleInput %02x", playerInput->participantId)
    switch (playerInput->inputType) {
    case ExamplePlayerInputTypeSelectTeam: {
        ExamplePlayer* player = &self->players[playerInput->participantId];
        spawnAvatarForPlayer(self, player);
    } break;
    case ExamplePlayerInputTypeInGame: {
        handleInGameInput(self, playerInput);
    }
    case ExamplePlayerInputTypeForced:
    case ExamplePlayerInputTypeWaitingForReconnect:
    case ExamplePlayerInputTypeNone:
        break;
    }
}

static void simulateSnake(ExampleGame* game, ExampleSnake* snake)
{
    // Check if eaten food
    if (snake->x[0] == game->food.x && snake->y[0] == game->food.y) {
        snake->length++;
        uint32_t randomX = pseudoRandomNext(&game->pseudoRandom);
        uint32_t randomY = pseudoRandomNext(&game->pseudoRandom);
        game->food.x = randomX % (WIDTH - 2) + 1;
        game->food.y = randomY % (HEIGHT - 2) + 1;
    }

    snakeMoveWithFront(snake);
    snakeMoveFrontUsingDirection(snake);

    for (int i = 1; i < snake->length; i++) {
        if (snake->x[i] == snake->x[0] && snake->y[i] == snake->y[0]) {
            game->gameIsOver = true;
            CLOG_INFO("ate yourself")
            return;
        }
    }

    if (snake->x[0] <= 0 || snake->x[0] >= WIDTH - 1 || snake->y[0] <= 0
        || snake->y[0] >= HEIGHT - 1) {
        game->gameIsOver = true;
        CLOG_INFO("collided with wall %d, %d", snake->x[0], snake->y[0])
        return;
    }
}

static void simulateSnakes(ExampleGame* game)
{
    for (size_t i = 0; i < game->snakeCount; ++i) {
        ExampleSnake* snake = &game->snakes[i];
        simulateSnake(game, snake);
    }
}

void exampleSimulationTick(
    ExampleGame* game, const ExamplePlayerInput* _input, const size_t inputCount)
{
    if (game->ticksBetweenMoves != 0) {
        game->ticksBetweenMoves--;
        return;
    }

    game->ticksBetweenMoves = 3;

    if (game->gameIsOver) {
        return;
    }

    for (size_t i = 0; i < inputCount; ++i) {
        const ExamplePlayerInput* playerInput = &_input[i];
        handleInput(game, playerInput);
    }

    simulateSnakes(game);
}
