/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>

#define WIDTH 40
#define HEIGHT 30

void exampleGameInit(ExampleGame* self)
{
    self->players.playerCount = 0;
    self->snakes.snakeCount = 0;

    self->food.position.x = 4;
    self->food.position.y = 4;
    self->pseudoRandom = 0;
    self->gameIsOver = false;
    self->ticksBetweenMoves = 10;

    self->area.height = HEIGHT;
    self->area.width = WIDTH;

    for (size_t i = 0; i < EXAMPLE_GAME_MAX_PARTICIPANTS; ++i) {
        self->participantLookup[i].isUsed = false;
    }
}
