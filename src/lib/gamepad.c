/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/gamepad.h>
#include <ncurses.h>

void exampleGamepadUpdate(ExampleGamepad* self)
{
    int ch = getch();

    ExampleGamepadState* state = &self->internalState;

    switch (ch) {
    case KEY_LEFT:
        state->horizontal = -1;
        break;
    case KEY_RIGHT:
        state->horizontal = 1;
        break;
    case KEY_UP:
        state->buttons = 1;
        break;
    case KEY_DOWN:
        state->buttons = 0;
        break;
    }
}

void exampleGamepadRead(ExampleGamepad* self, ExampleGamepadState* state)
{
    *state = self->internalState;
}
