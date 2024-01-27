/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/gamepad.h>
#include <ncurses.h>

void exampleGamepadInit(ExampleGamepad* self)
{
    self->internalState.horizontal = 0;
    self->internalState.vertical = 0;
    self->internalState.buttons = 0;
}

void exampleGamepadUpdate(ExampleGamepad* self)
{
    int ch = getch();

    ExampleGamepadState* state = &self->internalState;

    switch (ch) {
    case KEY_LEFT:
        state->horizontal = -1;
        state->vertical = 0;
        break;
    case KEY_RIGHT:
        state->horizontal = 1;
        state->vertical = 0;
        break;
    case KEY_UP:
        state->vertical = 1;
        state->horizontal = 0;
        break;
    case KEY_DOWN:
        state->vertical = -1;
        state->horizontal = 0;
        break;
    }
}

void exampleGamepadRead(ExampleGamepad* self, ExampleGamepadState* state)
{
    *state = self->internalState;
}
