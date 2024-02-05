/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>
#include <tiny-libc/tiny_libc.h>

#define WIDTH 50
#define HEIGHT 20

void exampleGameInit(ExampleGame* self)
{
    tc_mem_clear_type(self);
    self->food.position.x = 4;
    self->food.position.y = 4;
    self->ticksBetweenMoves = 10;
    self->area.height = HEIGHT;
    self->area.width = WIDTH;
}
