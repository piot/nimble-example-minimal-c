/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>
#include <example/input.h>
#include <example/simulation.h>
#include <clog/clog.h>

#define WIDTH 40
#define HEIGHT 30

static void snakeMoveWithFront(ExampleSnake* snake)
{
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }
}

static void snakeSetDirectionFromInput(ExampleSnake* snake, ExamplePlayerInGameInput* pad)
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

void exampleSimulationTick(ExampleGame* game, ExamplePlayerInput* input)
{
    if (game->ticksBetweenMoves != 0) {
        game->ticksBetweenMoves--;
        return;
    }

    game->ticksBetweenMoves = (uint32_t) (3 - (game->snake.length / 10));

    if (game->gameIsOver) {
        return;
    }

    switch (input->inputType) {
    case ExamplePlayerInputTypeInGame: {
        ExamplePlayerInGameInput* pad = &input->input.inGameInput;
        ExampleSnake* snake = &game->snake;

        // Check if eaten food
        if (snake->x[0] == game->food.x && snake->y[0] == game->food.y) {
            snake->length++;
            uint32_t randomX = pseudoRandomNext(&game->pseudoRandom);
            uint32_t randomY = pseudoRandomNext(&game->pseudoRandom);
            game->food.x = randomX % (WIDTH - 2) + 1;
            game->food.y = randomY % (HEIGHT - 2) + 1;
        }

        snakeSetDirectionFromInput(snake, pad);
        snakeMoveWithFront(&game->snake);
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

        break;
    }

    case ExamplePlayerInputTypeForced:
    case ExamplePlayerInputTypeWaitingForReconnect:
    case ExamplePlayerInputTypeNone:
    case ExamplePlayerInputTypeSelectTeam:
        break;
    }
}
