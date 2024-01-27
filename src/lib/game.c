/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>

void exampleGameInit(ExampleGame* self)
{
    self->snake.x[0] = 2;
    self->snake.y[0] = 2;
    self->snake.length = 1;

    self->food.x = 4;
    self->food.y = 4;
    self->pseudoRandom = 0;
    self->gameIsOver = false;
    self->ticksBetweenMoves = 10;
}
