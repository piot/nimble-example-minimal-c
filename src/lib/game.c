/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>

void exampleGameInit(ExampleGame* self)
{
    self->positionX = 0;
    self->positionY = 0;
    self->jumpTimer = 0;
}
