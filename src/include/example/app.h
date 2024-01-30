/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_APP_H
#define NIMBLE_EXAMPLE_MINIMAL_APP_H

#include <example/client.h>
#include <example/game_app.h>
#include <example/host.h>

typedef struct ExampleApp {
    ExampleClient client;
    ExampleGameApp combinedGame;
} ExampleApp;

#endif
