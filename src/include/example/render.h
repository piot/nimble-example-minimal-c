/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_RENDER_H
#define NIMBLE_EXAMPLE_MINIMAL_RENDER_H

#include <example/game.h>
typedef struct ExampleRender {
    int dummy;
} ExampleRender;

void exampleRenderInit(ExampleRender* self);
void exampleRenderUpdate(
    ExampleRender* self, ExampleGame* authoritativeGame, ExampleGame* predictedGame);
void exampleRenderClose(ExampleRender* self);

#endif
