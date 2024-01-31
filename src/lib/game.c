/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>

void exampleGameInit(ExampleGame* self)
{
    self->playerCount = 0;
    self->snakeCount = 0;

    self->food.x = 4;
    self->food.y = 4;
    self->pseudoRandom = 0;
    self->gameIsOver = false;
    self->ticksBetweenMoves = 10;
}

