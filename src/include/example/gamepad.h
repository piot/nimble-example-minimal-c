/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_GAMEPAD_H
#define NIMBLE_EXAMPLE_MINIMAL_GAMEPAD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ExampleGamepadState {
    int horizontal;
    int vertical;
    uint8_t buttons;
    bool debugPauseIsDown;
} ExampleGamepadState;

typedef struct ExampleGamepad {
    ExampleGamepadState internalState;
} ExampleGamepad;

void exampleGamepadInit(ExampleGamepad* self);
void exampleGamepadUpdate(ExampleGamepad* self);
void exampleGamepadRead(const ExampleGamepad* self, ExampleGamepadState* state);

#endif
