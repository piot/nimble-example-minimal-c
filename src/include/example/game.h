/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_GAME_H
#define NIMBLE_EXAMPLE_MINIMAL_GAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NIMBLE_EXAMPLE_SNAKE_MAX_LENGTH (20)

typedef enum ExampleDirection {
    ExampleDirectionUp,
    ExampleDirectionRight,
    ExampleDirectionDown,
    ExampleDirectionLeft,
} ExampleDirection;

typedef struct ExampleSnake {
    int x[NIMBLE_EXAMPLE_SNAKE_MAX_LENGTH];
    int y[NIMBLE_EXAMPLE_SNAKE_MAX_LENGTH];
    int length;
    ExampleDirection movementDirection;
} ExampleSnake;

typedef struct ExampleFood {
    int x;
    int y;
} ExampleFood;

typedef struct ExampleGame {
    ExampleSnake snake;
    ExampleFood food;
    uint32_t pseudoRandom;
    bool gameIsOver;
    uint32_t ticksBetweenMoves;
} ExampleGame;

void exampleGameInit(ExampleGame* self);

#endif
