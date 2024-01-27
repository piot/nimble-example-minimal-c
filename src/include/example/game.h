/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_GAME_H
#define NIMBLE_EXAMPLE_MINIMAL_GAME_H

#include <stddef.h>

typedef struct ExampleGame {
    int positionX;
    int positionY;
    size_t jumpTimer;
} ExampleGame;

void exampleGameInit(ExampleGame* self);

#endif

